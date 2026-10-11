#include "snav.h"
#include <stdio.h>
#include <stdlib.h>
int main(int argc,char **argv){
 Nav n;int i,skin=argc>2?atoi(argv[2]):0;int w=skin?SNAV_SKIN_W:SNAV_W,h=skin?SNAV_SKIN_H:SNAV_H;
 uint32_t *p=malloc(w*h*4);nav_init(&n,"INWARD");n.time=1.4;nav_render(&n,p,skin);
 FILE *f=fopen(argc>1?argv[1]:"preview.ppm","wb");if(!f)return 1;
 fprintf(f,"P6\n%d %d\n255\n",w,h);for(i=0;i<w*h;i++){fputc((p[i]>>16)&255,f);fputc((p[i]>>8)&255,f);fputc(p[i]&255,f);}fclose(f);free(p);return 0;
}
