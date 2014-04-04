#ifndef __UTIL_H
#define __UTIL_H

int curtime();
int curtime_u();
short int sign(double i);
#ifndef min
int min(int a, int b);
int max(int a, int b);
#endif
double clamp(double val, double min, double max);
double angnorm(double ang);
void fpsCounter();
void playSound(Mix_Chunk *snd);
int random_range(int min, int max);

#endif
