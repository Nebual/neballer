#ifndef __LEVEL_H
#define __LEVEL_H

extern int curLevel;
extern int balls;
extern Entity *ballInPlay;

void checkWinLoss();
int countBricks();
void generateLevel(int level);
void drawBackground(SDL_Renderer *renderer, double dt);

#endif