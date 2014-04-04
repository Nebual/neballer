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

static TextureData explosion2;
static Mix_Chunk *shootRocket1;
void initTextures() {
	explosion2.texture = IMG_LoadTexture(renderer, "res/explosion_50.png");
	explosion2.animMaxFrames = 36;
	explosion2.w = 50; explosion2.h = 50;
	explosion2.animWidth = 8;
	explosion2.animDuration = 2;

	shootRocket1 = Mix_LoadWAV("res/shootRocket1.ogg");
}

Entity* EntityCreate(char texturePath[], Type type, int x, int y) {
	Entity* this = (Entity *) malloc(sizeof(Entity));
	
	this->texture = IMG_LoadTexture(renderer, texturePath);
	if (!this->texture) {fprintf(stderr, "Couldn't load %s: %s\n", texturePath, SDL_GetError());}
	//if(type == TYPE_PLANET) {SDL_SetTextureBlendMode(this->texture, SDL_BLENDMODE_ADD);} // ??? For some reason, planets render better with add, yet nothing else does
	//if(this->type != TYPE_BALL && this->type != TYPE_PLAYER) {SDL_SetTextureBlendMode(this->texture, SDL_BLENDMODE_ADD);}
	SDL_SetTextureBlendMode(this->texture, SDL_BLENDMODE_BLEND);
	this->rect = (SDL_Rect) {x,y,0,0};
	int format;
	SDL_QueryTexture(this->texture, &format, NULL, &this->rect.w, &this->rect.h);
	//printf("Texture %s: %d\n", texturePath, format);
	
	//this->pos = (Vector) {x + this->rect.w/2,y + this->rect.w/2};
	this->pos = (Vector) {x,y};
	this->ang = 0;
	this->vel = (Vector) {0,0};
	this->avel = 0;
	this->thrust = 175;
	this->athrust = 100;
	this->maxSpeed = 200;
	this->type = type;
	this->collision = 0;
	this->collisionSize = (this->rect.w + this->rect.h) / 4; // Average of widthheight / 2
	this->damage = 0;
	this->health = 0;
	this->owner = NULL;
	this->deathTime = 0;
	this->animTime = 0;
	this->animDuration = 0;
	this->animMaxFrames = 0;
	switch(type) {
		case TYPE_PLAYER:
			this->collision = 1;
			break;
		case TYPE_BLOCK:
			this->collision = 1;
			this->health = 100;
			break;
		case TYPE_BALL:
			this->collision = 1;
			this->thrust = 250;
			this->maxSpeed = 800;
			this->damage = 100;
			this->collisionSize = min(this->rect.w, this->rect.h) / 2;
			break;
		//default:
	}
	
	ents[entsC] = this; entsC++;
	return this;
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
void EntityDraw(Entity *ent, SDL_Rect *camera, double dt) {
	ent->rect.x = ent->pos.x - camera->x;
	ent->rect.y = ent->pos.y - camera->y;
	int ret;
	if(ent->type == TYPE_BALL || ent->type == TYPE_PLAYER) { // For some reason, it didn't like switching between the methods on a per-entity basis (using ent->ang != 0)
		ret = SDL_RenderCopyEx(renderer, ent->texture, EntityGetFrame(ent, dt), &ent->rect, ent->ang, NULL, SDL_FLIP_NONE);
	} else{
		ret = SDL_RenderCopy(renderer, ent->texture, EntityGetFrame(ent, dt), &ent->rect);
	}
	if(ret != 0) {printf("Render failed: %s\n", SDL_GetError());}
}
void EntityUpdate(Entity *ent, double dt) {
	static struct timeval curtime;
	if(ent->deathTime != 0 && ent->deathTime < SDL_GetTicks()) {
		return EntityRemove(ent);
	}
	switch(ent->type) {
		case TYPE_BALL: {
			EntityMovement(ent, dt);
			
			Entity *hit = TestCollision(ent);
			if(hit != NULL) {
				if(DEBUG) printf("Collision Occured: PosX %.3f, PosY: %.3f, Block (%.2f, %.2f), CollisionSizes (%d, %d), distance: %.2f\n", ent->pos.x, ent->pos.y, hit->pos.x, hit->pos.y, ent->collisionSize, hit->collisionSize, EntityDistance(hit, ent));
				EntityDamage(hit, ent->damage);
				ent->vel.y = abs(ent->vel.y) * sign(ent->pos.y - hit->pos.y);
				
				if(hit->type == TYPE_PLAYER){
					ent->vel.x += hit->vel.x * 0.1;
				}
			}
			
			break;
		} case TYPE_PLAYER:
			EntityMovement(ent, dt);
			break;
	}
	if((ent->pos.x + ent->rect.w) > WIDTH){
		if(ent->type == TYPE_BALL){
			if(ent->vel.x > 0){
				ent->vel.x *= -1;
			}
		}else if(ent->type ==TYPE_PLAYER){
			ent->pos.x = WIDTH - ent->rect.w;
		}
	}
	else if(ent->pos.x < 0){
		if(ent->type == TYPE_BALL){
			if(ent->vel.x < 0){
				ent->vel.x *= -1;
			}
		}
		if(ent->type == TYPE_PLAYER){
			ent->pos.x = 0;
		}
	}
	
	if((ent->pos.y + ent->rect.h) > HEIGHT){
		if(ent->type == TYPE_BALL){
			EntityRemove(ent);
			ballInPlay = NULL;
		}
	}else if (ent->pos.y < 0){
		if(ent->vel.y < 0){
			ent->vel.y *= -1;
		}
	}
}
void EntityMovement(Entity *ent, double dt) {
	ent->ang += ent->avel * dt;
	ent->pos.x += ent->vel.x * dt;
	ent->pos.y += ent->vel.y * dt;
}
void EntityThrust(Entity *ent, double mul, double dt) {
	ent->vel.x += cos(ent->ang * (M_PI/180)) * ent->thrust * mul * dt;
	ent->vel.y += sin(ent->ang * (M_PI/180)) * ent->thrust * mul * dt;
	double speed = sqrt(ent->vel.x * ent->vel.x + ent->vel.y * ent->vel.y);
	if (speed > ent->maxSpeed) {
		ent->vel.x *= ent->maxSpeed / speed;
		ent->vel.y *= ent->maxSpeed / speed;
	}
}
Entity* TestCollision(Entity *ent) {
	for(int i=0; i<entsC; i++) {
		if(ents[i] == NULL || ents[i]->collision == 0 || ent == ents[i]) continue;
		if(ent->owner == ents[i] || ents[i]->type == TYPE_BALL) continue;
		
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
		ent->texture = explosion2.texture;
		ent->rect.w = explosion2.w; ent->rect.h = explosion2.h;
		ent->pos.y -= 12;
		ent->animDuration = explosion2.animDuration;
		ent->animMaxFrames = explosion2.animMaxFrames;
		ent->collision = 0;
		
		EntityDeathClock(ent, explosion2.animDuration * 1000);
	}
}
void EntityDeathClock(Entity *ent, int delay) {
	ent->deathTime = SDL_GetTicks() + delay;
}

void ShipBrake(Entity *ent, double dt) {
	double diff = ent->thrust * 0.7 * dt;
	ent->vel.x -= sign(ent->vel.x) * (diff > fabs(ent->vel.x) ? fabs(ent->vel.x) : diff);
	ent->vel.y -= sign(ent->vel.y) * (diff > fabs(ent->vel.y) ? fabs(ent->vel.y) : diff);
}

double EntityDistance(Entity *ent1, Entity *ent2) {
	return pow(pow(ent1->rect.x - ent2->rect.x, 2) + powf(ent1->rect.y - ent2->rect.y, 2), 0.5);
}

void GenBall(Entity *ent){
	if(ballInPlay != NULL || balls < 0) return;
	ballInPlay = EntityCreate("res/ball.png", TYPE_BALL, ent->pos.x, (ent->pos.y - 50));
	ballInPlay->vel.x = random_range(-150, 150);
	ballInPlay->vel.y = -300;
	balls--;
}
