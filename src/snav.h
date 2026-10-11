#ifndef SNAV_CORE_H
#define SNAV_CORE_H
#include <stdint.h>
#define SNAV_GRID 19
#define SNAV_W 287
#define SNAV_H 264
#define SNAV_SKIN_W 434
#define SNAV_SKIN_H 509
#define SNAV_NPCS 4
#define SNAV_PI 3.14159265358979323846
/* Standalone simulator: map layout is source-derived, NPC AI is simplified. */
typedef struct { double x,y,tx,ty,cooldown; int state; } NavNPC;
typedef struct {
 unsigned char rooms[SNAV_GRID][SNAV_GRID],found[SNAV_GRID][SNAV_GRID];
 uint32_t rng; char seed[96],notice[160];
 double x,y,yaw,time,blink,battery,eventTime; int mode,paused,power,mono,skin,coffinX,coffinY;
 NavNPC npc[SNAV_NPCS];
} Nav;
typedef struct { int forward,back,left,right,run,blink; } NavInput;
typedef struct { const char *name; double distance,radius; } NavContact;
void nav_init(Nav *n,const char *seed);
void nav_step(Nav *n,NavInput in,double dt);
void nav_trigger096(Nav *n);
uint32_t nav_seed(const char *seed);
int nav_room(const Nav *n,int x,int y);
int nav_contacts(const Nav *n,NavContact out[5]);
void nav_render(const Nav *n,uint32_t *pixels,int skin);
int nav_load_map(Nav *n,const char *path);
int nav_save_map(const Nav *n,const char *path);
#endif
