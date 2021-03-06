#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#include <getopt.h>

#include <SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include "util.h"
#include "entity.h"
#include "level.h"
#include "main.h"
#include "input.h"

SDL_Renderer *renderer;

Entity *ents[512];
int entsC = 0;

int WIDTH;
int HEIGHT;
int FIRSTLEVEL = 1;
Entity *ply;
int quit = 0;

void initVariables(int w, int h) {

}

int main(int argc, char *argv[]) {
	
	static SDL_Window *window;
	if(initWindow(&window, &renderer, argc, argv)) return 1;
	initVariables(WIDTH, HEIGHT);
	initTextures();
	initFonts();
	//initHUD();

	generateLevel(FIRSTLEVEL);
	
	ply = new Entity(TextureDataCreate("res/plank.png"), TYPE_PLAYER, WIDTH/2 - 50, HEIGHT - 36);
	
	int lastFrame = curtime_u() - 1;
	double dt;
	while(!quit) {
		if(DEBUG) fpsCounter();

		// Calculate dt
		int curFrame = curtime_u();
		if(lastFrame > curFrame) {dt = ((1000000 - lastFrame) + curFrame) / 1000000.0;}
		else {dt = (curFrame - lastFrame) / 1000000.0;}
		lastFrame = curFrame;

		// ===================
		// Update
		TimerRun();
		handleKeyboard(dt, ply);
		for(int enti=0; enti<entsC; enti++) {
			if(ents[enti] == NULL) continue;
			ents[enti]->Update(dt);
		}
		
		// ====================
		// Win/Loss conditions
		checkWinLoss();
		
		// ====================
		// Drawing
		drawBackground(renderer, dt);
		for(int enti=0; enti<entsC; enti++) {
			if(ents[enti] == NULL) continue;
			ents[enti]->Draw(dt);
		}
		drawHud(dt);

		// Flip render buffer
		SDL_RenderPresent(renderer);
		
		// Frame limiting, not needed if we're using vsync
		//SDL_Delay((1000 / 66) - (curtime_u() - lastFrame)/1000);
	}
	printf("\nShutting down...\n");

	// Destroy old textures
	for(int enti=0; enti<entsC; enti++) {
		if(ents[enti] == NULL) continue;
		delete ents[enti];
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	
	Mix_CloseAudio();
	Mix_Quit();

	IMG_Quit();
	SDL_Quit();
	return 0;
}


int initWindow(SDL_Window **window, SDL_Renderer **renderer, int argc, char *argv[]) {
	if(SDL_Init(SDL_INIT_EVERYTHING) == -1) {
		fprintf(stderr, "SDL Init failed: %s\n", SDL_GetError());
		return 1;
	}

	/*SDL_DisplayMode displayInfo;
	SDL_GetCurrentDisplayMode(0, &displayInfo); 
	WIDTH = displayInfo.w;
	HEIGHT = displayInfo.h;*/
	WIDTH = 800;
	HEIGHT = 600;
	
	int flags=IMG_INIT_JPG|IMG_INIT_PNG;
	if(IMG_Init(flags) != flags) {
		fprintf(stderr, "IMG_Init failed: %s\n", IMG_GetError());
		return 1;
	}

	if(Mix_OpenAudio( MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024 ) == -1) {
		fprintf(stderr, "Audio Init failed: %s\n", Mix_GetError());
		return 1;
	}
	int mixFlags = MIX_INIT_OGG;
	if(Mix_Init(mixFlags) != mixFlags) {
		fprintf(stderr, "Audio Init2 failed: %s\n", Mix_GetError());
		return 1;
	}
	
	if(TTF_Init()==-1) {
		printf("TTF_Init: %s\n", TTF_GetError());
		return 1;
	}

	int useSoftwareAccel = 0;
	int optionIndex = 0;
	int c;
	struct option longOptions[] = {
		{"software", no_argument, &useSoftwareAccel, 1},
		{"hardware", no_argument, &useSoftwareAccel, 0},
		{"level", required_argument, 0, 'l'}
	};

	while((c = getopt_long(argc, argv, "l", longOptions, &optionIndex)) != -1) {
		switch(c) {
			case 'l':
				FIRSTLEVEL = atoi(optarg);
				break;
		}
	}

	for(int i=0;i<SDL_GetNumRenderDrivers();i++) {
		SDL_RendererInfo info;
		SDL_GetRenderDriverInfo(i, &info);
		printf("Video Driver %d (%s): flags(%d) %s,%s,%s,%s - %d\n", i, info.name, info.flags, 
			(info.flags & SDL_RENDERER_SOFTWARE) ? "Software" : "",
			(info.flags & SDL_RENDERER_ACCELERATED) ? "Hardware Accelerated" : "",
			(info.flags & SDL_RENDERER_PRESENTVSYNC) ? "VSync" : "",
			(info.flags & SDL_RENDERER_TARGETTEXTURE) ? "TextureRender" : "",
			info.num_texture_formats
		);
		/*for(int texN=0; texN<info.num_texture_formats; texN++) {
			printf("%d ", info.texture_formats[texN]);
		}
		printf("\n");*/
	}
	
	*window = SDL_CreateWindow("Neballer", SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_FULLSCREEN_DESKTOP); // SDL_WINDOW_BORDERLESS
											//  -1 for first rendering driver that fits the flags
	if(useSoftwareAccel || (*renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)) == NULL) {
		fprintf(stderr, "Loading hardware acceleration failed. Using software fallback...\n");
		if((*renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_SOFTWARE)) == NULL) {
			fprintf(stderr, "Loading software fallback renderer failed! Error: %s\n", SDL_GetError());
			return 1;
		}
	}
	return 0;
}
