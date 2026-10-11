#include "../src/snav.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
static int connected(Nav *n){
 int seen[361]={0},q[361],h=0,t=0,total=0,x,y,i,dx[4]={1,-1,0,0},dy[4]={0,0,1,-1};
 for(y=0;y<19;y++)for(x=0;x<19;x++)if(nav_room(n,x,y))total++;
 q[t++]=16*19+9;seen[q[0]]=1;
 while(h<t){int v=q[h++];for(i=0;i<4;i++){x=v%19+dx[i];y=v/19+dy[i];if(nav_room(n,x,y)&&!seen[y*19+x]){seen[y*19+x]=1;q[t++]=y*19+x;}}}return t==total;
}
int main(void){
 Nav a,b;NavInput in={0};NavContact c[5];int i,j,count;char s[80];
 assert(nav_seed("abc")==('a'^('b'<<1)^('c'<<2)));
 for(i=0;i<1000;i++){
  sprintf(s,"seed-%d",i);nav_init(&a,s);nav_init(&b,s);
  assert(!memcmp(a.rooms,b.rooms,sizeof(a.rooms)));assert(connected(&a));
  in.forward=1;in.left=i%2;in.right=!in.left;
  for(j=0;j<300;j++){nav_step(&a,in,1.0/60);nav_step(&b,in,1.0/60);assert(nav_room(&a,(int)floor(a.x/8+0.5),(int)floor(a.y/8+0.5)));}
  assert(!memcmp(&a,&b,sizeof(a)));
 }
 nav_init(&a,"RADAR");for(i=0;i<4;i++){a.npc[i].x=a.x+100;a.npc[i].y=a.y;}
 a.npc[0].x=a.x+24;assert(nav_contacts(&a,c)==1);assert(c[0].distance==24&&c[0].radius==72);
 a.npc[0].x=a.x+24.01;assert(nav_contacts(&a,c)==0);
 a.npc[1].x=a.x+31.9;count=nav_contacts(&a,c);assert(count==1&&fabs(c[0].radius-47.85)<1e-6);
 a.mode=2;assert(nav_contacts(&a,c)==0);
 nav_init(&a,"MOVE");double y=a.y;in=(NavInput){1,0,0,0,0,0};for(i=0;i<60;i++)nav_step(&a,in,1.0/60);assert(fabs(a.y-(y-3))<1e-6);
 a.paused=1;y=a.y;nav_step(&a,in,1);assert(a.y==y);
 a.paused=0;a.power=0;nav_step(&a,in,1);assert(a.y==y);
 a.power=1;a.mode=1;a.battery=100;in=(NavInput){0};nav_step(&a,in,1);assert(fabs(a.battery-99.65)<1e-6);
 nav_trigger096(&a);assert(a.npc[2].state==1);for(i=0;i<500;i++)nav_step(&a,in,1.0/60);assert(a.npc[2].state==2);
 nav_init(&a,"MAP");assert(nav_save_map(&a,"/tmp/snav-test.snavmap"));nav_init(&b,"OTHER");assert(nav_load_map(&b,"/tmp/snav-test.snavmap"));assert(!memcmp(a.rooms,b.rooms,sizeof(a.rooms)));
 FILE *f=fopen("/tmp/snav-invalid.snavmap","w");fprintf(f,"SNAV-MAP-1\n#.................#\n");fclose(f);assert(!nav_load_map(&b,"/tmp/snav-invalid.snavmap"));
 uint32_t *p=malloc(434*509*4);nav_render(&a,p,1);nav_render(&a,p,0);free(p);
 puts("PASS: 1000 deterministic connected maps, movement, pause/power, battery, radar boundaries, 096 state, map import/export, rendering.");return 0;
}
