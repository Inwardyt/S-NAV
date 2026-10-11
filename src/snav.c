#include "snav.h"
#include "room_data.h"
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
static int roomzone(int y){return y>=12?0:y>=7?1:2;}
const char *nav_zone_name(int y){return roomzone(y)==0?"LCZ":roomzone(y)==1?"HCZ":"EZ";}
static int room_index(const char *name){unsigned i;for(i=0;i<ROOM_TEMPLATE_COUNT;i++)if(!strcmp(room_templates[i].name,name))return (int)i+1;return 0;}
static const RoomTemplate *room_template(const Nav *n,int x,int y){int id=n->roomId[y][x];return id>0&&(unsigned)id<=ROOM_TEMPLATE_COUNT?&room_templates[id-1]:NULL;}
const char *nav_room_name(const Nav *n,int x,int y){const RoomTemplate *r;if(!nav_room(n,x,y))return "outside facility";r=room_template(n,x,y);return r?r->name:"unassigned room";}
static int shape_at(const Nav *n,int x,int y){
 int left=nav_room(n,x-1,y),right=nav_room(n,x+1,y),up=nav_room(n,x,y-1),down=nav_room(n,x,y+1),sum=left+right+up+down;
 return sum==2&&!((left&&right)||(up&&down))?5:sum;
}
static int chance(Nav *n,int percent){return randrange(n,1,100)<=percent;}
static void assign_events(Nav *n){
 int x,y,firstLock=1,endCount=0,endChoice;memset(n->events,0,sizeof(n->events));memset(n->eventDone,0,sizeof(n->eventDone));
 n->coffinX=n->coffinY=-100;
 for(y=17;y>=0;y--)for(x=0;x<19;x++)if(nav_room(n,x,y)){
  const char *name=nav_room_name(n,x,y);int ev=0;
  if(!strcmp(name,"room2closets")||!strcmp(name,"room2testroom2")||!strcmp(name,"008"))ev|=1;
  if(!strcmp(name,"lockroom")||!strcmp(name,"lockroom_ez")){if(chance(n,30)||(firstLock&&chance(n,67)))ev|=1;firstLock=0;}
  if(!strcmp(name,"tunnel2")&&chance(n,20))ev|=1;
  if(!strcmp(name,"room2servers")||!strcmp(name,"lockroom2"))ev|=2;
  if(roomzone(y)==1){
   int prob=!strcmp(name,"room2pipes")||!strcmp(name,"tunnel2")?40:!strcmp(name,"room2pit")?50:!strcmp(name,"room4tunnels")||!strcmp(name,"room3z2")?70:!strcmp(name,"room4pit")||!strcmp(name,"room3pit")||!strcmp(name,"room3tunnel")||!strcmp(name,"tunnel")?60:0;
   if(prob&&chance(n,prob))ev|=2;
  }
  if(!strcmp(name,"room049"))ev|=4;
  if(!strcmp(name,"coffin")){n->coffinX=x;n->coffinY=y;}
  if(!strcmp(name,"endroom")&&roomzone(y)==0)endCount++;
  n->events[y][x]=(unsigned char)ev;
 }
 /* One selected endroom visual event, not a global immediate 106 pursuit. */
 endChoice=endCount?randrange(n,0,endCount):0;
 if(endChoice)for(y=17;y>=12;y--)for(x=0;x<19;x++)if(!strcmp(nav_room_name(n,x,y),"endroom")&&--endChoice==0)n->events[y][x]|=8;
}
static void assign_rooms(Nav *n){
 int x,y,z,sh,k,count,slots[361],id,total,pick,chosen,best,dist,idx,used[ROOM_TEMPLATE_COUNT]={0};unsigned i;
 memset(n->roomId,0,sizeof(n->roomId));
 for(y=0;y<19;y++)for(x=0;x<19;x++)if(nav_room(n,x,y)){
  sh=shape_at(n,x,y);z=roomzone(y);total=0;
  for(i=0;i<ROOM_TEMPLATE_COUNT;i++)if(room_templates[i].shape==sh&&(room_templates[i].zones&(1<<z)))total+=room_templates[i].weight;
  pick=total?randrange(n,1,total):0;id=0;
  for(i=0;i<ROOM_TEMPLATE_COUNT;i++)if(room_templates[i].shape==sh&&(room_templates[i].zones&(1<<z))){if(!id)id=(int)i+1;pick-=room_templates[i].weight;if(total&&pick<=0){id=(int)i+1;break;}}
  n->roomId[y][x]=(unsigned short)id;
 }
 if(nav_room(n,9,17))n->roomId[17][9]=(unsigned short)room_index("start");
 /* Reserve source set-room templates by shape, zone, priority and fraction. */
 for(z=0;z<3;z++)for(sh=1;sh<=5;sh++){
  count=0;for(y=17;y>=0;y--)for(x=0;x<19;x++)if(nav_room(n,x,y)&&roomzone(y)==z&&shape_at(n,x,y)==sh&&strcmp(nav_room_name(n,x,y),"start"))slots[count++]=y*19+x;
  memset(used,0,sizeof(used));
  for(k=0;k<count;k++){
   chosen=-1;for(i=0;i<ROOM_TEMPLATE_COUNT;i++)if(!used[i]&&room_templates[i].shape==sh&&(room_templates[i].zones&(1<<z))&&room_templates[i].position>=0&&(chosen<0||room_templates[i].priority>room_templates[chosen].priority))chosen=(int)i;
   if(chosen<0)break;
   used[chosen]=1;idx=mini((int)(room_templates[chosen].position*count),count-1);best=-1;dist=999;
   for(id=0;id<count;id++)if(slots[id]>=0&&abs(id-idx)<dist){best=id;dist=abs(id-idx);}
   if(best<0)break;
   y=slots[best]/19;x=slots[best]%19;n->roomId[y][x]=(unsigned short)(chosen+1);slots[best]=-1;
  }
 }
 for(y=6;y<=12;y+=6)for(x=0;x<19;x++)if(nav_room(n,x,y)&&nav_room(n,x,y-1)&&nav_room(n,x,y+1))n->roomId[y][x]=(unsigned short)room_index(y==12?"checkpoint1":"checkpoint2");
 assign_events(n);
}
static void activate(Nav *n,int which,int x,int y,double lifetime){
 NavNPC *a=&n->npc[which];memset(a,0,sizeof(*a));a->active=1;a->homeX=x;a->homeY=y;
 a->x=x*8;a->y=y*8-1.5;a->tx=a->x;a->ty=a->y;a->cooldown=which==0?5:0;a->lifetime=lifetime;
}
static void retire(Nav *n,int which){n->npc[which].active=0;n->npc[which].x=n->npc[which].y=-1000;if(which==0)n->next173=n->time+randrange(n,90,180);}
static void reset_events(Nav *n){
 int k;memset(n->found,0,sizeof(n->found));memset(n->eventDone,0,sizeof(n->eventDone));
 for(k=0;k<4;k++)retire(n,k);
 n->next106=randrange(n,12,17)*60.0;n->next173=90;n->lastRoomX=n->lastRoomY=-1;n->time=0;n->blink=0;
 n->found[cell(n->y)][cell(n->x)]=1;
}
void nav_init(Nav *n,const char *seed){
 memset(n,0,sizeof(*n));snprintf(n->seed,sizeof(n->seed),"%s",seed&&*seed?seed:"INWARD");
 n->rng=nav_seed(n->seed);if(!n->rng)n->rng=0x6D2B79F5;
 n->x=72;n->y=136;n->mode=3;n->power=1;n->battery=100;n->yaw=0;
 layout(n);assign_rooms(n);reset_events(n);
 snprintf(n->notice,sizeof(n->notice),"SIMULATION READY - room-based SCP events");
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
void nav_trigger096(Nav *n){
 NavNPC *a=&n->npc[2];if(!a->active||distance(n->x,n->y,a->x,a->y)>8){snprintf(n->notice,sizeof(n->notice),"No nearby SCP-096 to provoke");return;}
 if(a->state==0){a->state=1;a->cooldown=8;snprintf(n->notice,sizeof(n->notice),"SCP-096 provoked - chase begins in 8 seconds");}
}
int nav_can_stand(const Nav *n,double x,double y){
 int cx=cell(x),cy=cell(y),xx,yy;double nearestX,nearestY,dx,dy;
 if(!nav_room(n,cx,cy))return 0;
 for(yy=cy-1;yy<=cy+1;yy++)for(xx=cx-1;xx<=cx+1;xx++)if(!nav_room(n,xx,yy)){
  nearestX=fmax(xx*8-4,fmin(x,xx*8+4));nearestY=fmax(yy*8-4,fmin(y,yy*8+4));dx=x-nearestX;dy=y-nearestY;
  if(dx*dx+dy*dy<SNAV_PLAYER_RADIUS*SNAV_PLAYER_RADIUS-1e-9)return 0;
 }
 return 1;
}
static void enter_room(Nav *n,int x,int y){
 int ev=n->events[y][x],done=n->eventDone[y][x];const char *name=nav_room_name(n,x,y);
 if((ev&1)&&!(done&1)){activate(n,0,x,y,45);n->eventDone[y][x]|=1;snprintf(n->notice,sizeof(n->notice),"SCP-173 room event: %s",name);}
 if((ev&2)&&!(done&2)){if(!n->npc[2].active||n->npc[2].state==0)activate(n,2,x,y,0);n->eventDone[y][x]|=2;snprintf(n->notice,sizeof(n->notice),"SCP-096 room event: %s",name);}
 if((ev&4)&&!(done&4)){activate(n,3,x,y,0);n->eventDone[y][x]|=4;snprintf(n->notice,sizeof(n->notice),"SCP-049 chamber event (2D approximation)");}
 if((ev&8)&&!(done&8)&&!n->npc[1].active){activate(n,1,x,y,12);n->npc[1].state=3;n->eventDone[y][x]|=8;snprintf(n->notice,sizeof(n->notice),"SCP-106 scripted endroom appearance");}
 /* Rare LCZ relocation on entering a new room, with a long cooldown. */
 if(!n->npc[0].active&&n->time>=n->next173&&roomzone(y)==0&&strcmp(name,"start")&&strcmp(name,"checkpoint1")&&chance(n,8)){
  activate(n,0,x,y,35);n->next173=n->time+randrange(n,90,180);snprintf(n->notice,sizeof(n->notice),"Occasional SCP-173 LCZ relocation");
 }
}
void nav_step(Nav *n,NavInput in,double dt){
 int i,cx,cy,parts,p;double mx,my,step,d,speed;NavNPC *a;
 if(n->paused||!n->power||dt<=0)return;
 n->time+=dt;n->blink=fmax(0,n->blink-dt);
 if(in.blink||fmod(n->time,4.5)<dt)n->blink=0.25;
 n->yaw+=(in.right-in.left)*2.6*dt;n->yaw=fmod(n->yaw,2*SNAV_PI);
 speed=in.run?7.2:4.2;step=(in.forward-in.back)*speed*dt;parts=(int)ceil(fabs(step)/0.2);if(parts<1)parts=1;
 for(p=0;p<parts;p++){
  mx=n->x+sin(n->yaw)*step/parts;my=n->y-cos(n->yaw)*step/parts;
  if(nav_can_stand(n,mx,n->y))n->x=mx;
  if(nav_can_stand(n,n->x,my))n->y=my;
  cx=cell(n->x);cy=cell(n->y);n->found[cy][cx]=1;
  if(cx!=n->lastRoomX||cy!=n->lastRoomY){n->lastRoomX=cx;n->lastRoomY=cy;enter_room(n,cx,cy);}
 }
 if(n->mode!=3)n->battery=fmax(0,n->battery-0.35*dt);
 if(!n->npc[1].active){const RoomTemplate *r=room_template(n,cell(n->x),cell(n->y));if(r&&r->noDecals)n->next106+=dt;}
 if(!n->npc[1].active&&n->time>=n->next106){
  const RoomTemplate *r=room_template(n,cell(n->x),cell(n->y));
  if(!r||!r->noDecals){activate(n,1,cell(n->x),cell(n->y),45);n->npc[1].x=n->x;n->npc[1].y=n->y+5;n->next106=n->time+45+randrange(n,22000,27000)/70.0;snprintf(n->notice,sizeof(n->notice),"SCP-106 timer expired");}
 }
 for(i=0;i<4;i++){
  a=&n->npc[i];if(!a->active)continue;
  if(a->lifetime>0){a->lifetime-=dt;if(a->lifetime<=0){retire(n,i);continue;}}
  a->cooldown=fmax(0,a->cooldown-dt);d=distance(n->x,n->y,a->x,a->y);
  if(i==2){if(!a->state)continue;if(a->state==1){if(a->cooldown>0)continue;a->state=2;}}
  if(i==1&&a->state==3)continue;
  if(i==0&&(n->blink<=0&&looking(n,a)))continue;
  if(i!=2&&a->cooldown>0)continue;
  if(i==3&&(roomzone(cell(n->y))!=1||d>16))continue;
  speed=i==0?5:i==1?1.15:i==2?7.0:1.65;
  if(i==1){a->tx=n->x;a->ty=n->y;}
  else if(distance(a->x,a->y,a->tx,a->ty)<0.05)target(n,a,1);
  mx=a->tx-a->x;my=a->ty-a->y;d=hypot(mx,my);step=fmin(speed*dt,d);
  if(d>0.001){a->x+=mx/d*step;a->y+=my/d*step;}
  if(distance(n->x,n->y,a->x,a->y)<0.7){
   const char *names[]={"173","106","096","049"};
   if(i==2){a->state=0;a->x=a->homeX*8;a->y=a->homeY*8;a->tx=a->x;a->ty=a->y;}
   else if(i==3){a->x=a->homeX*8;a->y=a->homeY*8;a->tx=a->x;a->ty=a->y;a->cooldown=15;}
   else retire(n,i);
   snprintf(n->notice,sizeof(n->notice),"SCP-%s encounter - returned to event/cooldown",names[i]);
  }
 }
}
int nav_contacts(const Nav *n,NavContact out[5]){
 int i,count=0;double d;const char *names[]={"SCP-173","SCP-106","SCP-096","SCP-049"};
 if(n->mode!=3||!n->power)return 0;
 for(i=0;i<4;i++){
  if(!n->npc[i].active)continue;
  d=distance(n->x,n->y,n->npc[i].x,n->npc[i].y);if(i==0)d=ceil(d/8)*8;
  if(d<32){out[count].name=names[i];out[count].distance=d;out[count++].radius=d*(i==0?3:1.5);}
 }
 d=distance(n->x,n->y,n->coffinX*8,n->coffinY*8);
 if(cell(n->x)==n->coffinX&&cell(n->y)==n->coffinY&&d<8){out[count].name="SCP-895";out[count].distance=d;out[count++].radius=6+3*sin(n->time*7);}
 return count;
}
int nav_save_map(const Nav *n,const char *path){
 int x,y;FILE *f=fopen(path,"w");if(!f)return 0;
 fprintf(f,"SNAV-MAP-2\n");for(y=0;y<19;y++){for(x=0;x<19;x++)fputc(nav_room(n,x,y)?'#':'.',f);fputc('\n',f);}
 for(y=0;y<19;y++)for(x=0;x<19;x++)if(nav_room(n,x,y)&&n->roomId[y][x])fprintf(f,"%d %d %s\n",x,y,nav_room_name(n,x,y));
 return fclose(f)==0;
}
int nav_load_map(Nav *n,const char *path){
 char line[128];int x,y,count=0,sx=0,sy=0,q[361],seen[361]={0},h=0,t=0,k;
 unsigned char rooms[19][19];unsigned short ids[19][19]={0};int tagged;FILE *f=fopen(path,"r");if(!f)return 0;
 if(!fgets(line,sizeof(line),f)||(strncmp(line,"SNAV-MAP-1",10)&&strncmp(line,"SNAV-MAP-2",10))){fclose(f);return 0;}
 tagged=line[9]=='2';
 for(y=0;y<19;y++){
  if(!fgets(line,sizeof(line),f)||strlen(line)<19){fclose(f);return 0;}
  for(x=0;x<19;x++){if(line[x]!='#'&&line[x]!='.'){fclose(f);return 0;}rooms[y][x]=line[x]=='#';if(rooms[y][x]){count++;sx=x;sy=y;}}
 }
 if(tagged)while(fgets(line,sizeof(line),f)){char name[80];int id;
  if(sscanf(line,"%d %d %79s",&x,&y,name)!=3||x<0||x>=19||y<0||y>=19||!rooms[y][x]||(id=room_index(name))==0){fclose(f);return 0;}ids[y][x]=(unsigned short)id;
 }
 fclose(f);if(!count)return 0;
 q[t++]=sy*19+sx;seen[q[0]]=1;
 while(h<t){int v=q[h++],dx[4]={1,-1,0,0},dy[4]={0,0,1,-1};for(k=0;k<4;k++){
  x=v%19+dx[k];y=v/19+dy[k];if(x>=0&&y>=0&&x<19&&y<19&&rooms[y][x]&&!seen[y*19+x]){seen[y*19+x]=1;q[t++]=y*19+x;}
 }}
 if(t!=count)return 0;
 memcpy(n->rooms,rooms,sizeof(rooms));assign_rooms(n);
 if(tagged){for(y=0;y<19;y++)for(x=0;x<19;x++)if(ids[y][x])n->roomId[y][x]=ids[y][x];assign_events(n);}
 for(y=0;y<19;y++)for(x=0;x<19;x++)if(!strcmp(nav_room_name(n,x,y),"start")){sx=x;sy=y;}
 n->x=sx*8;n->y=sy*8;n->yaw=0;reset_events(n);
 snprintf(n->notice,sizeof(n->notice),tagged?"Imported room grid and source room names":"Imported room grid; room names estimated");return 1;
}
