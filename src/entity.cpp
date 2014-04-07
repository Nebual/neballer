#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#ifdef _WIN32
	#include <windows.h>
#endif

#include <SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include "util.h"
#include "entity.h"
#include "main.h"
#include "level.h"

TextureData ballTD;
TextureData blockTDs[127];
TextureData explosionTD;
static Mix_Chunk *bounceSound;
static Mix_Chunk *hitSounds[3];
static Mix_Chunk *failSound;
void initTextures() {
	ballTD = TextureDataCreate("res/ball.png");
	blockTDs[BLOCK_NORMAL] = TextureDataCreate("res/block.png");
	blockTDs[BLOCK_SPEEDUP] = TextureDataCreate("res/block_speedup.png");
	blockTDs[BLOCK_SLOWDOWN] = TextureDataCreate("res/block_slowdown.png");
	blockTDs[BLOCK_RANDOM] = TextureDataCreate("res/block_random.png");

	explosionTD.texture = IMG_LoadTexture(renderer, "res/explosion_50.png");
	explosionTD.animMaxFrames = 36;
	explosionTD.w = 50; explosionTD.h = 50;
	explosionTD.animWidth = 8;
	explosionTD.animDuration = 2;

	bounceSound = Mix_LoadWAV("res/sounds/bounce.ogg");
	hitSounds[0] = Mix_LoadWAV("res/sounds/hit1.wav");
	hitSounds[1] = Mix_LoadWAV("res/sounds/hit2.wav");
	hitSounds[2] = Mix_LoadWAV("res/sounds/hit3.wav");
	failSound = Mix_LoadWAV("res/sounds/fail.ogg");
}

TextureData TextureDataCreate(const char texturePath[]) {
	TextureData data = {NULL, 0, 0, 0, 0, 0};
	data.texture = IMG_LoadTexture(renderer, texturePath);
	if (!data.texture) {fprintf(stderr, "Couldn't load %s: %s\n", texturePath, SDL_GetError());}
	SDL_SetTextureBlendMode(data.texture, SDL_BLENDMODE_BLEND);
	SDL_QueryTexture(data.texture, NULL, NULL, &data.w, &data.h);
	
	return data;
}

Entity* EntityCreate(TextureData texdata, Type type, int x, int y) {
	Entity* ent = (Entity *) malloc(sizeof(Entity));
	
	ent->texture = texdata.texture;
	ent->rect = (SDL_Rect) {x,y,texdata.w,texdata.h};
	ent->pos = (Vector) {x,y};
	ent->vel = (Vector) {0,0};
	ent->type = type;
	ent->blockType = BLOCK_NONE;
	ent->collision = 0;
	ent->collisionSize = (ent->rect.w + ent->rect.h) / 4; // Average of widthheight / 2
	ent->damage = 0;
	ent->health = 0;
	ent->deathTime = 0;
	ent->animTime = 0;
	ent->animDuration = 0;
	ent->animMaxFrames = 0;
	switch(type) {
		case TYPE_PLAYER:
			ent->collision = 1;
			break;
		case TYPE_BLOCK:
			ent->collision = 1;
			ent->health = 100;
			break;
		case TYPE_BALL:
			ent->collision = 1;
			ent->damage = 100;
			break;
	}
	
	ents[entsC] = ent; entsC++;
	return ent;
}
void EntityRemove(Entity *ent) {
	// TODO: These should probably utilize an internal entID and rlID, but then GC would be harder
	for(int i=0; i<entsC; i++) {
		if(ents[i] == ent) {ents[i] = NULL; break;}
	}
	//SDL_DestroyTexture(ent->texture); // We're caching now
	free(ent);
}
void EntityGC() {
	// HO BOY
	//Garbage collector
	int newC = 0;
	for(int i=0; i<entsC; i++) {
		if(ents[i] != NULL) {
			ents[newC] = ents[i];
			if(i != newC) {ents[i] = NULL;}
			newC++;
		}
	}
	entsC = newC;
}
SDL_Rect* EntityGetFrame(Entity *ent, double dt) {
	if(ent->animDuration != 0) {
		static SDL_Rect srcRect;
		static int curFrame;
		
		ent->animTime += dt;
		curFrame = ent->animMaxFrames * (ent->animTime / ent->animDuration);
		//printf("Test (%.4f, %.4f, %.4f, %d, %d)\n", ent->animTime, ent->animDuration, ent->animTime / ent->animDuration, ent->animMaxFrames, curFrame);
		srcRect = (SDL_Rect) {ent->rect.w * (curFrame % 8), ent->rect.h * (curFrame / 8), ent->rect.w, ent->rect.h};
		//if(ent->animFrame > ent->animFrameMax*4) {ent->animFrame = 0;}
		return &srcRect;
	}
	return NULL;
}
void EntityDraw(Entity *ent, double dt) {
	ent->rect.x = ent->pos.x;
	ent->rect.y = ent->pos.y;
	int ret;
	//if(ent->type == TYPE_BALL || ent->type == TYPE_PLAYER) { // For some reason, it didn't like switching between the methods on a per-entity basis (using ent->ang != 0)
	//	ret = SDL_RenderCopyEx(renderer, ent->texture, EntityGetFrame(ent, dt), &ent->rect, ent->ang, NULL, SDL_FLIP_NONE);
	//} else{
		ret = SDL_RenderCopy(renderer, ent->texture, EntityGetFrame(ent, dt), &ent->rect);
	//}
	if(ret != 0) {printf("Render failed: %s\n", SDL_GetError());}
}
void EntityUpdate(Entity *ent, double dt) {
	if(ent->deathTime != 0 && ent->deathTime < SDL_GetTicks()) {
		return EntityRemove(ent);
	}
	switch(ent->type) {
		case TYPE_BALL: {
			EntityMovement(ent, dt);
			
			if((ent->pos.x + ent->rect.w) > WIDTH) {
				if(ent->vel.x > 0) {
					ent->vel.x *= -1;
				}
			} else if(ent->pos.x < 0) {
				if(ent->vel.x < 0) {
					ent->vel.x *= -1;
				}
			}
			if((ent->pos.y + ent->rect.h) > HEIGHT) {
				EntityRemove(ent);
				ballInPlay = NULL;
				playSound(failSound);
			} else if (ent->pos.y < 0){
				if(ent->vel.y < 0){
					ent->vel.y *= -1;
				}
			}
			
			Entity *hit = TestCollision(ent);
			if(hit != NULL) {
				if(DEBUG) printf("Collision Occured: PosX %.3f, PosY: %.3f, Block (%.2f, %.2f), CollisionSizes (%d, %d), distance: %.2f\n", ent->pos.x, ent->pos.y, hit->pos.x, hit->pos.y, ent->collisionSize, hit->collisionSize, EntityDistance(hit, ent));
				ent->vel.y = abs(ent->vel.y) * sign(ent->pos.y - hit->pos.y);
				
				if(hit->type == TYPE_PLAYER){
					ent->vel.x += hit->vel.x * 0.25;
					playSound(bounceSound);
				} else if(hit->type == TYPE_BLOCK) {
					playSound(hitSounds[rand() % 3]);
					switch(hit->blockType) {
						case BLOCK_SPEEDUP: 
							ent->vel.x *= 1.25;
							ent->vel.y *= 1.1;
							break;
						case BLOCK_SLOWDOWN: 
							ent->vel.x /= 1.25;
							ent->vel.y /= 1.1;
							break;
						case BLOCK_RANDOM:
							ent->vel.x = ent->vel.x * 0.5 + random_range(-250, 250);
							break;
					}
				}
				EntityDamage(hit, ent->damage);
			}
			
			break;
		} case TYPE_PLAYER:
			EntityMovement(ent, dt);
			if((ent->pos.x + ent->rect.w) > WIDTH) {
				ent->pos.x = WIDTH - ent->rect.w;
			}
			else if(ent->pos.x < 0) {
				ent->pos.x = 0;
			}
			break;
	}
}
void EntityMovement(Entity *ent, double dt) {
	ent->pos.x += ent->vel.x * dt;
	ent->pos.y += ent->vel.y * dt;
}
Entity* TestCollision(Entity *ent) {
	for(int i=0; i<entsC; i++) {
		if(ents[i] == NULL || ents[i]->collision == 0 || ent == ents[i]) continue;
		if(ents[i]->type == TYPE_BALL) continue;
		
		// Rect collisions
		double maxX = max(ents[i]->pos.x, ent->pos.x);
		double minX = min(ents[i]->pos.x + ents[i]->rect.w, ent->pos.x + ent->rect.w);
		double maxY = max(ents[i]->pos.y, ent->pos.y);
		double minY = min(ents[i]->pos.y + ents[i]->rect.h, ent->pos.y + ent->rect.h);
		if(maxX < minX && maxY < minY) {return ents[i];}
		
		// Happy fun Circle collisions :D :D
		//if(EntityDistance(ents[i], ent) < (ents[i]->collisionSize + ent->collisionSize)) {return ents[i];}
	}
	return NULL;
}

void EntityDamage(Entity *ent, int damage) {
	if(ent->health <= 0) return;
	ent->health -= damage;
	if(ent->health <= 0) {
		ent->type = TYPE_EXPLOSION;
		ent->texture = explosionTD.texture;
		ent->rect.w = explosionTD.w; ent->rect.h = explosionTD.h;
		ent->pos.y -= 12;
		ent->animDuration = explosionTD.animDuration;
		ent->animMaxFrames = explosionTD.animMaxFrames;
		ent->collision = 0;
		
		EntityDeathClock(ent, explosionTD.animDuration * 1000);
	}
}
void EntityDeathClock(Entity *ent, int delay) {
	ent->deathTime = SDL_GetTicks() + delay;
}

double EntityDistance(Entity *ent1, Entity *ent2) {
	return pow(pow(ent1->rect.x - ent2->rect.x, 2) + powf(ent1->rect.y - ent2->rect.y, 2), 0.5);
}

void GenBall(Entity *ent){
	if(ballInPlay != NULL || balls < 0) return;
	ballInPlay = EntityCreate(ballTD, TYPE_BALL, ent->pos.x, (ent->pos.y - 50));
	ballInPlay->vel.x = random_range(-150, 150);
	ballInPlay->vel.y = -300;
	balls--;
}
