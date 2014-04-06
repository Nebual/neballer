#ifndef __ENTITY_H
#define __ENTITY_H

typedef struct {
	double x, y;
} Vector;

typedef enum {
	TYPE_NONE,
	TYPE_BLOCK,
	TYPE_PLAYER,
	TYPE_BALL,
	TYPE_EXPLOSION,
	TYPE_MAX
} Type;

typedef enum {
	BLOCK_NONE = ' ',
	BLOCK_NORMAL = '=',
	BLOCK_SPEEDUP = '+',
	BLOCK_SLOWDOWN = '-',
	BLOCK_RANDOM = 'X'
} BlockType;

typedef struct entity {
	Vector vel;
	Vector pos;
	SDL_Rect rect;
	SDL_Texture *texture;
	Uint32 deathTime;
	short int collision;
	short int collisionSize;	// Used by anything involved with collisions. Define size for circle collision check 
	short int damage;			// Projectiles
	
	double animTime;
	double animDuration;
	short int animMaxFrames;

	short int health;			// Ships

	Type type;
	BlockType blockType;
} Entity;

typedef struct {
	SDL_Texture *texture;
	short int animMaxFrames;
	short int animWidth;
	short int animDuration;
	int w;
	int h;
} TextureData;


extern TextureData blockTDs[127];


void initTextures();
TextureData TextureDataCreate(char texturePath[]);
Entity* EntityCreate(TextureData texdata, Type type, int x,int y);
void EntityRemove(Entity *ent);
void EntityDraw(Entity *ent, double dt);
void EntityUpdate(Entity *ent, double dt);
void EntityMovement(Entity *ent, double dt);
void EntityGC();
Entity* TestCollision(Entity *ent);
void EntityDamage(Entity *ent, int damage);
void EntityDeathClock(Entity * ent, int delay);
void GenBall(Entity *ent);
double EntityDistance(Entity *ent1, Entity *ent2);


#endif
