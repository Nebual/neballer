#include <stdio.h>
#include <sys/time.h>
#include <math.h>

#include <SDL2/SDL_mixer.h>

#include "util.h"

void fpsCounter() {	
	static int curSec;
	static int frames = 0;
	if(curtime() != curSec) {
		curSec = curtime();
		printf("FPS: %d\n", frames);
		frames = 0;
	}
	frames++;
}

inline short int sign(double i) {
	return i > 0 ? 1 : -1;
}

inline int min(int a, int b) {
	return a < b ? a : b;
}
inline int max(int a, int b) {
	return a > b ? a : b;
}

inline double clamp(double val, double min, double max) {
	if(val < min) return min;
	if(val > max) return max;
	return val;
}

inline double angnorm(double ang) {
	return fmod(ang + 180, 360) - 180;
}

int curtime() {
	static struct timeval curtime;
	gettimeofday(&curtime, NULL);
	return curtime.tv_sec;
}
int curtime_u() {
	static struct timeval curtime;
	gettimeofday(&curtime, NULL);
	return curtime.tv_usec;
}

void playSound(Mix_Chunk *snd) {
	#ifndef __arm__
	if(snd == NULL || Mix_PlayChannel(-1, snd, 0 ) == -1) {printf("Snd loading error: %s\n", Mix_GetError());}
	#endif
}

