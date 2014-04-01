#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#include <getopt.h>

#include <SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

//#include "util.h"
#include "entity.h"
#include "main.h"
#include "input.h"

void handleKeyboard(double dt, Entity *ply) {
	static int keysPressed[255];
	
	static SDL_Event keyevent;
	while(SDL_PollEvent(&keyevent)) {
		switch(keyevent.type) {
			case SDL_KEYDOWN:
				keysPressed[keyevent.key.keysym.scancode] = 1;
				if(DEBUG) printf("Key pressed: %d\n", keyevent.key.keysym.scancode);
				
				switch(keyevent.key.keysym.sym) {
					case SDLK_c:
					case SDLK_q:
						if(keyevent.key.keysym.mod & KMOD_CTRL) {quit = 1;}
						break;
					case SDLK_h:
						if(DEBUG) printf("Starting GC, old entsC: %d", entsC);
						EntityGC();
						if(DEBUG) printf(", new entsC: %d\n", entsC);
						break;
				}
				break;
			case SDL_KEYUP:
				keysPressed[keyevent.key.keysym.scancode] = 0;
				
				switch(keyevent.key.keysym.sym) {
				}
		}
	}
	
	if(keysPressed[SDL_SCANCODE_W] || keysPressed[SDL_SCANCODE_UP]) {
		EntityThrust(ply, 1, dt);
	}
	if(keysPressed[SDL_SCANCODE_S] || keysPressed[SDL_SCANCODE_DOWN]) {
		EntityThrust(ply, -1, dt);
	}
	ply->avel = 0;
	if(keysPressed[SDL_SCANCODE_A] || keysPressed[SDL_SCANCODE_LEFT]) {
		ply->avel = -ply->athrust;
	}
	else if(keysPressed[SDL_SCANCODE_D] || keysPressed[SDL_SCANCODE_RIGHT]) {
		ply->avel = ply->athrust;
	}
	if(keysPressed[SDL_SCANCODE_SPACE]) {
		//ShipShoot(ply);
	}

	//camera->x += keysPressed[SDL_SCANCODE_L] - keysPressed[SDL_SCANCODE_J];
	//camera->y += keysPressed[SDL_SCANCODE_K] - keysPressed[SDL_SCANCODE_I];
}