#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#include <getopt.h>

#include <SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include "entity.h"
#include "main.h"
#include "level.h"
#include "util.h"

int curLevel;
int balls = 3;
Entity *ballInPlay;

void checkWinLoss(){
	if(balls <= 0 && ballInPlay == NULL){
		printf("You've dropped the bar far too many times to bother trying again. You lose!\n");
		quit = 1;
	} else if (countBricks() == 0){
		Mix_Chunk *victorySound = Mix_LoadWAV("res/sounds/victory.ogg");
		playSound(victorySound);
		
		delete ballInPlay;
		balls++;
		
		Entity::GC();
		curLevel++;
		generateLevel(curLevel);
	}
}

int countBricks(){
	int bricks = 0;
	for(int i=0; i < entsC; i++){
		if(ents[i] == NULL) continue;
		if(ents[i]->type == TYPE_BLOCK){
			bricks++;
		}
	}
	return bricks;
}

void generateLevel(int level) {
	curLevel = level;
	char filename[5] = "";
	char line[18] = "";
	sprintf(filename, "levels/%d.lvl", level);
	if(DEBUG) printf("Reading file %s\n", filename);
	
	FILE *fp = fopen(filename, "r");
	if(!fp) {
		printf("%s not found, you've completed the last level!\n", filename);
		quit = 1;
	}
	
	for(int y=0; fgets(line, sizeof(line), fp); y++) {
		if(DEBUG) printf("Read line: %s", line);
		for(int x=0; x<16; x++) {
			if(blockTDs[line[x]].texture != NULL) {
				if(DEBUG) printf("Spawning block(%d,%c)\n", line[x], line[x]);
				Entity *ent = new Entity(blockTDs[line[x]], TYPE_BLOCK, x*50, y*25);
				ent->blockType = (BlockType) line[x];
			}
		}
		memset(line, '\0', sizeof(line));
	}
}

void drawBackground(SDL_Renderer *renderer, double dt) {
	const SDL_Rect rect = {0,0,WIDTH,HEIGHT};
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
	SDL_SetRenderDrawColor(renderer, 20,20,20, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &rect);
}
