#include "../src/snav.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
static int connected(Nav *n){
 int seen[361]={0},q[361],h=0,t=0,total=0,x,y,i,dx[4]={1,-1,0,0},dy[4]={0,0,1,-1};
 for(y=0;y<19;y++)for(x=0;x<19;x++)if(nav_room(n,x,y))total++;
 q[t++]=17*19+9;seen[q[0]]=1;
 while(h<t){int v=q[h++];for(i=0;i<4;i++){x=v%19+dx[i];y=v/19+dy[i];if(nav_room(n,x,y)&&!seen[y*19+x]){seen[y*19+x]=1;q[t++]=y*19+x;}}}return t==total;
}
static void visit(Nav *n,int x,int y){n->x=x*8;n->y=y*8;n->lastRoomX=n->lastRoomY=-1;nav_step(n,(NavInput){0},1.0/60);}
int main(void){
 Nav a,b;NavInput in={0};NavContact c[5];int i,j,x,y,count,found;char s[80];
 assert(nav_seed("abc")==('a'^('b'<<1)^('c'<<2)));
 for(i=0;i<1000;i++){
  sprintf(s,"seed-%d",i);nav_init(&a,s);nav_init(&b,s);
  assert(!memcmp(a.rooms,b.rooms,sizeof(a.rooms)));assert(!memcmp(a.roomId,b.roomId,sizeof(a.roomId)));assert(connected(&a));
  assert(!strcmp(nav_room_name(&a,9,17),"start"));assert(a.next106>=720&&a.next106<=1020);assert(nav_contacts(&a,c)==0);
  for(y=12;y<19;y++)for(x=0;x<19;x++)assert(!(a.events[y][x]&2)&&!(a.events[y][x]&4));
  in=(NavInput){1,0,i%2,!(i%2),0,0};
  for(j=0;j<300;j++){nav_step(&a,in,1.0/60);nav_step(&b,in,1.0/60);assert(nav_can_stand(&a,a.x,a.y));}
  assert(!memcmp(&a,&b,sizeof(a)));
 }
 nav_init(&a,"RADAR");a.npc[0].active=1;a.npc[0].x=a.x+24;a.npc[0].y=a.y;
 assert(nav_contacts(&a,c)==1);assert(c[0].distance==24&&c[0].radius==72);
 a.npc[0].x=a.x+24.01;assert(nav_contacts(&a,c)==0);
 a.npc[1].active=1;a.npc[1].x=a.x+31.9;a.npc[1].y=a.y;count=nav_contacts(&a,c);assert(count==1&&fabs(c[0].radius-47.85)<1e-6);
 a.mode=2;assert(nav_contacts(&a,c)==0);
 nav_init(&a,"MOVE");double oldY=a.y;in=(NavInput){1,0,0,0,0,0};for(i=0;i<60;i++)nav_step(&a,in,1.0/60);assert(fabs(a.y-(oldY-4.2))<1e-6);
 a.paused=1;oldY=a.y;nav_step(&a,in,1);assert(a.y==oldY);
 a.paused=0;a.power=0;nav_step(&a,in,1);assert(a.y==oldY);
 a.power=1;a.mode=1;a.battery=100;in=(NavInput){0};nav_step(&a,in,1);assert(fabs(a.battery-99.65)<1e-6);
 /* No fabricated 096: pressing Enter in LCZ does not create one. */
 nav_init(&a,"INWARD");nav_trigger096(&a);assert(!a.npc[2].active);
 found=0;for(y=0;y<12&&!found;y++)for(x=0;x<19&&!found;x++)if(a.events[y][x]&2){visit(&a,x,y);found=1;}
 assert(found&&a.npc[2].active&&!a.npc[2].state);nav_trigger096(&a);assert(a.npc[2].state==1);
 for(i=0;i<500;i++)nav_step(&a,(NavInput){0},1.0/60);
 /* After a close encounter 096 returns to its original room, not random LCZ. */
 assert(a.npc[2].active&&a.npc[2].homeY<12);
 /* Initial 106 timer, no automatic early pursuit while staying in start. */
 nav_init(&a,"INWARD");double initialDelay=a.next106;for(i=0;i<60*180;i++)nav_step(&a,(NavInput){0},1.0/60);assert(!a.npc[1].active);assert(fabs(a.next106-a.time-initialDelay)<1e-6);
 found=0;for(y=7;y<12&&!found;y++)for(x=0;x<19&&!found;x++)if(nav_room(&a,x,y)&&!strcmp(nav_room_name(&a,x,y),"tunnel")){visit(&a,x,y);found=1;}
 if(!found){for(y=12;y<17&&!found;y++)for(x=0;x<19&&!found;x++)if(nav_room(&a,x,y)&&!strcmp(nav_room_name(&a,x,y),"room2")){visit(&a,x,y);found=1;}}
 assert(found);a.next106=a.time;nav_step(&a,(NavInput){0},1.0/60);assert(a.npc[1].active);
 /* The whole 6-pixel arrow fits: 2.25-world-unit circle against wall AABBs. */
 nav_init(&a,"COLLISION");memset(a.rooms,0,sizeof(a.rooms));memset(a.events,0,sizeof(a.events));a.rooms[9][9]=1;a.x=a.y=72;a.lastRoomX=a.lastRoomY=9;
 assert(nav_can_stand(&a,72,72));assert(!nav_can_stand(&a,74,72));
 for(i=0;i<360;i++){a.yaw=i*SNAV_PI/180;nav_step(&a,(NavInput){1,0,0,0,1,0},0.2);assert(nav_can_stand(&a,a.x,a.y));}
 assert(a.x>=70.25&&a.x<=73.75&&a.y>=70.25&&a.y<=73.75);
 a.rooms[9][10]=1;assert(nav_can_stand(&a,76,72));assert(!nav_can_stand(&a,76,74));
 a.rooms[10][9]=1;assert(!nav_can_stand(&a,76,76));
 nav_init(&a,"MAP");assert(nav_save_map(&a,"/tmp/snav-test.snavmap"));nav_init(&b,"OTHER");assert(nav_load_map(&b,"/tmp/snav-test.snavmap"));
 assert(!memcmp(a.rooms,b.rooms,sizeof(a.rooms)));assert(!memcmp(a.roomId,b.roomId,sizeof(a.roomId)));
 FILE *legacy=fopen("/tmp/snav-legacy.snavmap","w");assert(legacy);fprintf(legacy,"SNAV-MAP-1\n");
 for(y=0;y<19;y++){for(x=0;x<19;x++)fputc(nav_room(&a,x,y)?'#':'.',legacy);fputc('\n',legacy);}fclose(legacy);
 assert(nav_load_map(&b,"/tmp/snav-legacy.snavmap"));assert(!memcmp(a.rooms,b.rooms,sizeof(a.rooms)));
 FILE *disconnected=fopen("/tmp/snav-disconnected.snavmap","w");assert(disconnected);fprintf(disconnected,"SNAV-MAP-1\n");
 for(y=0;y<19;y++){for(x=0;x<19;x++)fputc((y==0&&(x==0||x==18))?'#':'.',disconnected);fputc('\n',disconnected);}fclose(disconnected);
 assert(!nav_load_map(&b,"/tmp/snav-disconnected.snavmap"));
 FILE *f=fopen("/tmp/snav-invalid.snavmap","w");fprintf(f,"SNAV-MAP-1\n#.................#\n");fclose(f);assert(!nav_load_map(&b,"/tmp/snav-invalid.snavmap"));
 uint32_t *p=malloc(434*509*4);nav_render(&a,p,1);nav_render(&a,p,0);free(p);
 puts("PASS: 1000 deterministic connected maps + room catalogs, no LCZ 096/049 spawn events, dormant NPCs, 106 timers, movement, circle/wall/corner collision, radar thresholds, tagged map import/export, rendering.");return 0;
}
