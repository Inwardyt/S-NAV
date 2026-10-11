#include "snav.h"
#include "assets.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
static uint32_t lcd[SNAV_W*SNAV_H];
static void pixel(int x,int y,uint32_t c){if(x>=8&&x<=278&&y>=20&&y<=250)lcd[y*SNAV_W+x]=c;}
static void line(int x,int y,int xx,int yy,uint32_t c){
 int dx=abs(xx-x),sx=x<xx?1:-1,dy=-abs(yy-y),sy=y<yy?1:-1,err=dx+dy,e;
 while(1){pixel(x,y,c);if(x==xx&&y==yy)break;e=2*err;if(e>=dy){err+=dy;x+=sx;}if(e<=dx){err+=dx;y+=sy;}}
}
static void text(int x,int y,const char *s,uint32_t c){
 int xx,yy,ch;while(*s){ch=(unsigned char)*s++;if(ch<32||ch>127)ch='?';
 for(yy=0;yy<16;yy++)for(xx=0;xx<10;xx++)if(nav_glyphs[ch-32][yy]&(1<<xx))pixel(x+xx,y+yy,c);
 x+=8;}
}
static void circle(int cx,int cy,int r,uint32_t c){
 int x=r,y=0,err=1-r;if(r<1)return;
 while(x>=y){pixel(cx+x,cy+y,c);pixel(cx+y,cy+x,c);pixel(cx-y,cy+x,c);pixel(cx-x,cy+y,c);pixel(cx-x,cy-y,c);pixel(cx-y,cy-x,c);pixel(cx+y,cy-x,c);pixel(cx+x,cy-y,c);y++;if(err<0)err+=2*y+1;else{x--;err+=2*(y-x)+1;}}
}
void nav_render(const Nav *n,uint32_t *pixels,int skin){
 int x,y,i,x0,y0,cx=143,cy=128;NavContact contacts[5];int count=nav_contacts(n,contacts);
 uint32_t ink=0x1e1e1e,red=n->mono?ink:0x640000;
 for(y=0;y<SNAV_H;y++)for(x=0;x<SNAV_W;x++){
  uint32_t c=skin_palette[skin_pixels[(y+43)*434+x+74]];
  if(!n->power)c=0x37443b;
  lcd[y*SNAV_W+x]=c;
 }
 if(n->power){
  line(8,20,278,20,ink);line(278,20,278,250,ink);line(278,250,8,250,ink);line(8,250,8,20,ink);
  if(n->battery>0||n->mode==3){
   double coffin=hypot(n->x-n->coffinX*8,n->y-n->coffinY*8);
   for(y=0;y<19;y++)for(x=0;x<19;x++)if(nav_room(n,x,y)&&(n->mode>=2||n->found[y][x])){
    if(coffin<16&&((x*31+y*17+(int)(n->time*13))%16)>coffin)continue;
    x0=(int)lround(cx+(x*8-n->x)*3)-12;y0=(int)lround(cy+(y*8-n->y)*3)-12;
    if(x0>290||x0< -24||y0>264||y0< -24)continue;
    if(!nav_room(n,x-1,y))line(x0,y0,x0,y0+24,ink);
    if(!nav_room(n,x+1,y))line(x0+24,y0,x0+24,y0+24,ink);
    if(!nav_room(n,x,y-1))line(x0,y0,x0+24,y0,ink);
    if(!nav_room(n,x,y+1))line(x0,y0+24,x0+24,y0+24,ink);
   }
   if(fmod(n->time,1.0)>0.3){
    int ax[3],ay[3];double a[3]={n->yaw,n->yaw-140*SNAV_PI/180,n->yaw+140*SNAV_PI/180};
    for(i=0;i<3;i++){int r=i?5:6;ax[i]=(int)lround(cx+sin(a[i])*r);ay[i]=(int)lround(cy-cos(a[i])*r);}
    for(i=0;i<3;i++)line(ax[i],ay[i],ax[(i+1)%3],ay[(i+1)%3],n->mode==0?red:ink);
    if(n->mode<2)text(13,22,"MAP DATABASE OFFLINE",n->mode==0?red:ink);
   }
   if(n->mode==3&&fmod(n->time,0.6)<0.4)for(i=0;i<count;i++){
    circle(cx,cy-7,(int)lround(contacts[i].radius),red);text(13,43+i*20,contacts[i].name,red);
   }
  }
  if(n->mode!=3){
   /* Battery is above the map border, as in the game. */
   int bars=(int)ceil(n->battery/10),xx,yy;
   for(yy=1;yy<=14;yy++)for(xx=198;xx<=277;xx++)if(yy==1||yy==14||xx==198||xx==277)lcd[yy*SNAV_W+xx]=ink;
   for(i=0;i<bars;i++)for(yy=4;yy<12;yy++)for(xx=201+i*7;xx<206+i*7;xx++)lcd[yy*SNAV_W+xx]=ink;
  }
 }
 if(skin){
  for(i=0;i<434*509;i++)pixels[i]=skin_palette[skin_pixels[i]];
  for(y=0;y<SNAV_H;y++)memcpy(pixels+(y+43)*434+74,lcd+y*SNAV_W,SNAV_W*sizeof(uint32_t));
 }else memcpy(pixels,lcd,sizeof(lcd));
}
