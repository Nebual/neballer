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


void generateLevel() {
	for(int y=0; y<3; y++) {
		for(int x=0; x<20; x++) {
			EntityCreate("res/block.jpg", TYPE_BLOCK, x*32, y*22);
		}
	}
}

void drawBackground(SDL_Renderer *renderer, double dt) {
	const SDL_Rect rect = {0,0,WIDTH,HEIGHT};
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
	SDL_SetRenderDrawColor(renderer, 20,20,20, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &rect);
}
