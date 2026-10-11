#include "snav.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
static int mini(int a,int b){return a<b?a:b;}
static int maxi(int a,int b){return a>b?a:b;}
static double distance(double x,double y,double a,double b){return hypot(x-a,y-b);}
uint32_t nav_seed(const char *s){uint32_t h=0;int shift=0;while(*s){h^=((uint32_t)(unsigned char)*s++)<<shift;shift=(shift+1)%24;}return h;}
static uint32_t random32(Nav *n){uint32_t x=n->rng;x^=x<<13;x^=x>>17;x^=x<<5;return n->rng=x;}
static int randrange(Nav *n,int lo,int hi){if(hi<lo)hi=lo;return lo+(int)(random32(n)%(unsigned)(hi-lo+1));}
int nav_room(const Nav *n,int x,int y){return x>=0&&y>=0&&x<SNAV_GRID&&y<SNAV_GRID&&n->rooms[y][x];}
static int zone(int y){return mini((18-y)*3/18,2);}
static void layout(Nav *n){
 int x=9,y=16,i,k,x2,y2,width,height,halls,temp=9,th,passes=0;
 n->rooms[16][9]=n->rooms[17][9]=1;
 do{
  width=randrange(n,10,15);
  if(x>10.8)width=-width;else if(x>7.2)x-=width/2;
  if(x+width>15)width=15-x;else if(x+width<2)width=2-x;
  x=mini(x,x+width);width=abs(width);
  for(i=x;i<=x+width;i++)if(i>=0&&i<19)n->rooms[y][i]=1;
  height=randrange(n,3,4);if(y-height<1)height=y-1;
  halls=randrange(n,4,5);if(zone(y-height)!=zone(y-height+1))height--;
  temp=x;
  for(k=1;k<=halls;k++){
   x2=maxi(mini(randrange(n,x,x+width-1),16),2);
   while(x2<18&&(nav_room(n,x2,y-1)||nav_room(n,x2-1,y-1)||nav_room(n,x2+1,y-1)))x2++;
   if(x2<x+width){
    if(k==1){th=height;x2=randrange(n,1,2)==1?x:x+width;}else th=randrange(n,1,height);
    for(y2=y-th;y2<=y;y2++)n->rooms[y2][x2]=1;
    if(th==height)temp=x2;
   }
  }
  /* Keep every band connected, including rare skipped first branches. */
  for(y2=y-height;y2<=y;y2++)n->rooms[y2][temp]=1;
  x=temp;y-=height;
 }while(y>=2&&++passes<30);
 /* Source-inspired dead ends; full ForceRooms/CreateRooms not ported. */
 for(y=2;y<16;y++)for(x=2;x<16;x++)if(!nav_room(n,x,y)){
  int neighbours=nav_room(n,x+1,y)+nav_room(n,x-1,y)+nav_room(n,x,y+1)+nav_room(n,x,y-1);
  if(neighbours==1&&randrange(n,0,7)==0)n->rooms[y][x]=1;
 }
}
static int cell(double a){return (int)floor(a/8.0+0.5);}
static void spawn(Nav *n,NavNPC *a,double minDist,double maxDist){
 int x,y,t,bx=9,by=16;double d;
 for(t=0;t<1200;t++){
  x=randrange(n,1,17);y=randrange(n,1,17);
  if(!nav_room(n,x,y))continue;
  bx=x;by=y;d=distance(n->x,n->y,x*8,y*8);
  if(d>=minDist&&d<=maxDist)break;
 }
 a->x=bx*8;a->y=by*8;a->tx=a->x;a->ty=a->y;a->cooldown=8;a->state=0;
}
void nav_init(Nav *n,const char *seed){
 int x,y;memset(n,0,sizeof(*n));snprintf(n->seed,sizeof(n->seed),"%s",seed&&*seed?seed:"INWARD");
 n->rng=nav_seed(n->seed);if(!n->rng)n->rng=0x6D2B79F5;
 n->x=72;n->y=128;n->mode=3;n->power=1;n->battery=100;n->yaw=0;
 layout(n);n->found[16][9]=1;
 spawn(n,&n->npc[0],16,24);spawn(n,&n->npc[1],40,64);spawn(n,&n->npc[2],40,80);spawn(n,&n->npc[3],64,96);
 n->coffinX=9;n->coffinY=1;
 for(y=2;y<14;y++)for(x=2;x<16;x++)if(nav_room(n,x,y)&&nav_room(n,x+1,y)+nav_room(n,x-1,y)+nav_room(n,x,y+1)+nav_room(n,x,y-1)==1){n->coffinX=x;n->coffinY=y;}
 snprintf(n->notice,sizeof(n->notice),"SIMULATION READY - arrows to explore");
}
static int clearline(const Nav *n,double x,double y,double a,double b){
 int i,steps=(int)(distance(x,y,a,b)*2)+1;
 for(i=0;i<=steps;i++)if(!nav_room(n,cell(x+(a-x)*i/steps),cell(y+(b-y)*i/steps)))return 0;
 return 1;
}
static int looking(const Nav *n,const NavNPC *a){
 double dx=a->x-n->x,dy=a->y-n->y,d=hypot(dx,dy);
 return d<20&&d>0.01&&(dx*sin(n->yaw)-dy*cos(n->yaw))/d>0.55&&clearline(n,n->x,n->y,a->x,a->y);
}
static void target(Nav *n,NavNPC *a,int pursue){
 int sx=cell(a->x),sy=cell(a->y),gx=cell(n->x),gy=cell(n->y);
 int queue[361],prev[361],head=0,tail=0,start=sy*19+sx,goal=gy*19+gx,i,v,next;
 int dx[4]={1,-1,0,0},dy[4]={0,0,1,-1},choices[4],count=0;
 if(!nav_room(n,sx,sy))return;
 if(!pursue){
  for(i=0;i<4;i++)if(nav_room(n,sx+dx[i],sy+dy[i]))choices[count++]=i;
  if(count){i=choices[randrange(n,0,count-1)];a->tx=(sx+dx[i])*8;a->ty=(sy+dy[i])*8;}return;
 }
 for(i=0;i<361;i++)prev[i]=-1;
 queue[tail++]=start;prev[start]=start;
 while(head<tail&&prev[goal]<0){
  v=queue[head++];for(i=0;i<4;i++){
   int xx=v%19+dx[i],yy=v/19+dy[i];next=yy*19+xx;
   if(nav_room(n,xx,yy)&&prev[next]<0){prev[next]=v;queue[tail++]=next;}
  }
 }
 if(prev[goal]<0)return;
 if(start==goal){a->tx=n->x;a->ty=n->y;return;}
 next=goal;while(prev[next]!=start)next=prev[next];a->tx=(next%19)*8;a->ty=(next/19)*8;
}
void nav_trigger096(Nav *n){if(n->npc[2].state==0){n->npc[2].state=1;n->npc[2].cooldown=8;snprintf(n->notice,sizeof(n->notice),"SCP-096 provoked - chase begins in 8 seconds");}}
void nav_step(Nav *n,NavInput in,double dt){
 int i;double mx,my,step,d,speed;NavNPC *a;
 if(n->paused||!n->power)return;
 n->time+=dt;n->blink=fmax(0,n->blink-dt);
 if(in.blink||fmod(n->time,4.5)<dt)n->blink=0.25;
 n->yaw+=(in.right-in.left)*1.9*dt;n->yaw=fmod(n->yaw,2*SNAV_PI);
 speed=in.run?5.8:3.0;step=(in.forward-in.back)*speed*dt;
 mx=n->x+sin(n->yaw)*step;my=n->y-cos(n->yaw)*step;
 if(nav_room(n,cell(mx+0.2),cell(n->y))&&nav_room(n,cell(mx-0.2),cell(n->y)))n->x=mx;
 if(nav_room(n,cell(n->x),cell(my+0.2))&&nav_room(n,cell(n->x),cell(my-0.2)))n->y=my;
 n->found[cell(n->y)][cell(n->x)]=1;
 if(n->mode!=3)n->battery=fmax(0,n->battery-0.35*dt);
 for(i=0;i<4;i++){
  a=&n->npc[i];a->cooldown=fmax(0,a->cooldown-dt);d=distance(n->x,n->y,a->x,a->y);
  if(i==2){if(!a->state)continue;if(a->state==1){if(a->cooldown>0)continue;a->state=2;}}
  if(i==0&&(n->blink<=0&&looking(n,a)))continue;
  if(i!=2&&a->cooldown>0)continue;
  speed=i==0?14:i==1?1.15:i==2?7.0:1.65;
  if(i==1){a->tx=n->x;a->ty=n->y;}
  else if(distance(a->x,a->y,a->tx,a->ty)<0.05)target(n,a,i!=3||d<24);
  mx=a->tx-a->x;my=a->ty-a->y;d=hypot(mx,my);step=fmin(speed*dt,d);
  if(d>0.001){a->x+=mx/d*step;a->y+=my/d*step;}
  if(distance(n->x,n->y,a->x,a->y)<0.7){
   const char *names[]={"173","106","096","049"};
   spawn(n,a,24,48);a->cooldown=12;
   snprintf(n->notice,sizeof(n->notice),"SCP-%s encounter - reset for continued exploration",names[i]);
  }
 }
}
int nav_contacts(const Nav *n,NavContact out[5]){
 int i,count=0;double d;const char *names[]={"SCP-173","SCP-106","SCP-096","SCP-049"};
 if(n->mode!=3||!n->power)return 0;
 for(i=0;i<4;i++){
  d=distance(n->x,n->y,n->npc[i].x,n->npc[i].y);if(i==0)d=ceil(d/8)*8;
  if(d<32){out[count].name=names[i];out[count].distance=d;out[count++].radius=d*(i==0?3:1.5);}
 }
 d=distance(n->x,n->y,n->coffinX*8,n->coffinY*8);
 if(cell(n->x)==n->coffinX&&cell(n->y)==n->coffinY&&d<8){out[count].name="SCP-895";out[count].distance=d;out[count++].radius=6+3*sin(n->time*7);}
 return count;
}
int nav_save_map(const Nav *n,const char *path){
 int x,y;FILE *f=fopen(path,"w");if(!f)return 0;
 fprintf(f,"SNAV-MAP-1\n");for(y=0;y<19;y++){for(x=0;x<19;x++)fputc(nav_room(n,x,y)?'#':'.',f);fputc('\n',f);}return fclose(f)==0;
}
int nav_load_map(Nav *n,const char *path){
 char line[128];int x,y,count=0,sx=0,sy=0,q[361],seen[361]={0},h=0,t=0,k;
 unsigned char rooms[19][19];FILE *f=fopen(path,"r");if(!f)return 0;
 if(!fgets(line,sizeof(line),f)||strncmp(line,"SNAV-MAP-1",10)){fclose(f);return 0;}
 for(y=0;y<19;y++){
  if(!fgets(line,sizeof(line),f)||strlen(line)<19){fclose(f);return 0;}
  for(x=0;x<19;x++){if(line[x]!='#'&&line[x]!='.'){fclose(f);return 0;}rooms[y][x]=line[x]=='#';if(rooms[y][x]){count++;sx=x;sy=y;}}
 }
 fclose(f);if(!count)return 0;
 q[t++]=sy*19+sx;seen[q[0]]=1;
 while(h<t){int v=q[h++],dx[4]={1,-1,0,0},dy[4]={0,0,1,-1};for(k=0;k<4;k++){
  x=v%19+dx[k];y=v/19+dy[k];if(x>=0&&y>=0&&x<19&&y<19&&rooms[y][x]&&!seen[y*19+x]){seen[y*19+x]=1;q[t++]=y*19+x;}
 }}
 if(t!=count)return 0;
 memcpy(n->rooms,rooms,sizeof(rooms));memset(n->found,0,sizeof(n->found));n->x=sx*8;n->y=sy*8;n->yaw=0;n->time=0;
 n->found[sy][sx]=1;n->coffinX=sx;n->coffinY=sy;
 for(k=0;k<4;k++)spawn(n,&n->npc[k],16,40);
 snprintf(n->notice,sizeof(n->notice),"Imported room grid (NPCs remain simulated)");return 1;
}
