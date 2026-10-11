#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "snav.h"
static Nav nav;
static uint32_t frame[434*509];
static HWND wnd,seedBox,genButton,randomButton,modeBox,status,roomLabel;
#ifdef SNAV_SCREEN_ONLY
static const int screenOnly=1;
#else
static const int screenOnly=0;
#endif
static char configPath[MAX_PATH],configuredSeed[96]="INWARD";
static HDC backDC;static HBITMAP backBitmap,originalBitmap;static int backWidth,backHeight;
static int configuredFullscreen;
static void config_init(void){
 char *slash,model[32];GetModuleFileNameA(NULL,configPath,MAX_PATH);slash=strrchr(configPath,'\\');if(slash)slash[1]=0;
 if(strlen(configPath)+10<MAX_PATH)strcat(configPath,"S-NAV.ini");
 GetPrivateProfileStringA("navigator","seed","INWARD",configuredSeed,sizeof(configuredSeed),configPath);
 nav_init(&nav,configuredSeed);GetPrivateProfileStringA("navigator","model","ultimate",model,sizeof(model),configPath);
 nav.mode=!strcmp(model,"300")?1:!strcmp(model,"310")?2:!strcmp(model,"damaged")?0:3;
 nav.mono=GetPrivateProfileIntA("navigator","monochrome",0,configPath)!=0;
 configuredFullscreen=GetPrivateProfileIntA("navigator","fullscreen",0,configPath)!=0;
}
static void config_save(void){
 const char *models[]={"damaged","300","310","ultimate"};
 WritePrivateProfileStringA("navigator","seed",nav.seed,configPath);
 WritePrivateProfileStringA("navigator","model",models[nav.mode],configPath);
 WritePrivateProfileStringA("navigator","monochrome",nav.mono?"1":"0",configPath);
}
static void drop_buffer(void){if(backDC){SelectObject(backDC,originalBitmap);DeleteObject(backBitmap);DeleteDC(backDC);}backDC=NULL;backBitmap=NULL;backWidth=backHeight=0;}
static int prepare_buffer(HDC dc,int w,int h){
 if(w<=0||h<=0)return 0;
 if(backDC&&w==backWidth&&h==backHeight)return 1;
 drop_buffer();backDC=CreateCompatibleDC(dc);if(!backDC)return 0;
 backBitmap=CreateCompatibleBitmap(dc,w,h);if(!backBitmap){DeleteDC(backDC);backDC=NULL;return 0;}
 originalBitmap=(HBITMAP)SelectObject(backDC,backBitmap);backWidth=w;backHeight=h;return 1;
}
static int held[256],mouseKey,fullscreen=0;
static RECT previous,displayRect;static LONG oldStyle;
static DWORD lastTime;static double accumulator=0;
#define ID_SEED 101
#define ID_GEN 102
#define ID_RANDOM 103
#define ID_MODE 104
#define ID_SKIN 201
#define ID_MONO 202
#define ID_PAUSE 203
#define ID_RESET 204
#define ID_BATTERY 205
#define ID_LOAD 206
#define ID_SAVE 207
#define ID_HELP 208
#define ID_FULLSCREEN 209
static const char *modes[]={"S-NAV (damaged)","S-NAV 300","S-NAV 310","S-NAV Ultimate"};
static void clear_keys(void){memset(held,0,sizeof(held));mouseKey=0;}
static void generate(int randomSeed){
 char seed[96];int mode=nav.mode,skin=nav.skin,mono=nav.mono;clear_keys();
 if(randomSeed){sprintf(seed,"%08lX%04lX",(unsigned long)GetTickCount(),(unsigned long)(rand()&65535));if(seedBox)SetWindowTextA(seedBox,seed);}else if(seedBox)GetWindowTextA(seedBox,seed,sizeof(seed));else snprintf(seed,sizeof(seed),"%s",nav.seed);
 nav_init(&nav,seed);nav.mode=mode;nav.skin=screenOnly?0:skin;nav.mono=mono;accumulator=0;config_save();
 SetFocus(wnd);InvalidateRect(wnd,NULL,FALSE);
}
static void layout_controls(void){
 if(!wnd)return;
 RECT r;GetClientRect(wnd,&r);int w=r.right,h=r.bottom;
 if(screenOnly)return;
 int bw=65,rw=68,mw=154,ew=w-bw-rw-mw-40;if(ew<65)ew=65;
 MoveWindow(seedBox,8,8,ew,25,TRUE);MoveWindow(genButton,ew+14,8,bw,25,TRUE);
 MoveWindow(randomButton,ew+bw+19,8,rw,25,TRUE);MoveWindow(modeBox,ew+bw+rw+24,8,mw,160,TRUE);
 MoveWindow(status,10,h-70,w-20,65,TRUE);MoveWindow(roomLabel,10,39,w-20,22,TRUE);
 ShowWindow(seedBox,fullscreen?SW_HIDE:SW_SHOW);ShowWindow(genButton,fullscreen?SW_HIDE:SW_SHOW);
 ShowWindow(randomButton,fullscreen?SW_HIDE:SW_SHOW);ShowWindow(modeBox,fullscreen?SW_HIDE:SW_SHOW);ShowWindow(status,fullscreen?SW_HIDE:SW_SHOW);ShowWindow(roomLabel,fullscreen?SW_HIDE:SW_SHOW);
}
static void toggle_fullscreen(void){
 if(!fullscreen){MONITORINFO mi;mi.cbSize=sizeof(mi);GetWindowRect(wnd,&previous);oldStyle=GetWindowLongA(wnd,GWL_STYLE);
 GetMonitorInfoA(MonitorFromWindow(wnd,MONITOR_DEFAULTTONEAREST),&mi);SetWindowLongA(wnd,GWL_STYLE,WS_POPUP|WS_VISIBLE);SetMenu(wnd,NULL);
 SetWindowPos(wnd,HWND_TOP,mi.rcMonitor.left,mi.rcMonitor.top,mi.rcMonitor.right-mi.rcMonitor.left,mi.rcMonitor.bottom-mi.rcMonitor.top,SWP_FRAMECHANGED);fullscreen=1;
 }else{SetWindowLongA(wnd,GWL_STYLE,oldStyle);SetMenu(wnd,(HMENU)GetPropA(wnd,"NAVMENU"));fullscreen=0;
 SetWindowPos(wnd,NULL,previous.left,previous.top,previous.right-previous.left,previous.bottom-previous.top,SWP_NOZORDER|SWP_FRAMECHANGED);}
 layout_controls();InvalidateRect(wnd,NULL,TRUE);
}
static void map_dialog(int save){
 OPENFILENAMEA ofn;char path[MAX_PATH]="facility.snavmap";int was=nav.paused;nav.paused=1;clear_keys();
 memset(&ofn,0,sizeof(ofn));ofn.lStructSize=sizeof(ofn);ofn.hwndOwner=wnd;ofn.lpstrFilter="S-NAV room grid\0*.snavmap\0All files\0*.*\0";
 ofn.lpstrFile=path;ofn.nMaxFile=MAX_PATH;ofn.lpstrDefExt="snavmap";ofn.Flags=OFN_NOCHANGEDIR|(save?OFN_OVERWRITEPROMPT:OFN_FILEMUSTEXIST);
 if(save?GetSaveFileNameA(&ofn):GetOpenFileNameA(&ofn)){
  int ok=save?nav_save_map(&nav,path):nav_load_map(&nav,path);
  if(!ok)MessageBoxA(wnd,save?"Could not write the file.":"Invalid map. Use a connected 19 x 19 grid of # and . after SNAV-MAP-1.","S-NAV",MB_OK|MB_ICONERROR);
 }
 nav.paused=was;SetFocus(wnd);
}
static void command(int id){
 switch(id){
 case ID_SKIN:if(!screenOnly)nav.skin=!nav.skin;break;case ID_MONO:nav.mono=!nav.mono;config_save();break;
 case ID_PAUSE:nav.paused=!nav.paused;clear_keys();break;case ID_RESET:generate(0);break;
 case ID_BATTERY:nav.battery=100;nav.power=1;break;case ID_LOAD:map_dialog(0);break;case ID_SAVE:map_dialog(1);break;
 case ID_FULLSCREEN:toggle_fullscreen();break;
 case ID_HELP:clear_keys();MessageBoxA(wnd,
 "Left / Right: turn\nUp / Down: forward / backward\nShift: move faster\nSpace: blink (173 can move)\nEnter: provoke nearby 096\nTab: pause\nD: device / LCD view\nM: monochrome / game red contacts\nP: power\nB: fresh battery\nR: restart this seed\nN: random seed\nF11: full screen, Esc: leave full screen\n\nUltimate is the default and has SCP distance rings.\n300 reveals visited rooms. 310 reveals the full map.\nOnly Ultimate detects SCPs.\n\nNPCs appear through room events and timers.\n106 starts with a 12-17 minute timer.\nMap seeds are repeatable SIMULATOR seeds, not exact game maps.\nNPC AI and encounters are simplified; no sound, combat, or game assets needed.","S-NAV controls",MB_OK);break;
 }
 SetFocus(wnd);InvalidateRect(wnd,NULL,FALSE);
}
static void update_status(void){
 char s[512],label[256];static char previousStatus[512],previousRoom[256];NavContact c[5];int count=nav_contacts(&nav,c);
 if(screenOnly)return;
 sprintf(s,"%s  |  %s  |  Room %d, %d  |  %d contacts  |  Battery %s\r\nArrows: turn / move   Space: blink   Enter: provoke nearby 096   Tab: pause   D: device view   F11: full screen\r\n%s",
 modes[nav.mode],!nav.power?"OFF":nav.paused?"PAUSED":"SIMULATION",(int)floor(nav.x/8+0.5),(int)floor(nav.y/8+0.5),count,nav.mode==3?"unlimited":nav.battery<=0?"empty":"active",nav.notice);
 if(strcmp(previousStatus,s)){SetWindowTextA(status,s);strcpy(previousStatus,s);}
 snprintf(label,sizeof(label),"Current room: %s | %s | grid %d, %d",nav_zone_name((int)floor(nav.y/8+0.5)),nav_room_name(&nav,(int)floor(nav.x/8+0.5),(int)floor(nav.y/8+0.5)),(int)floor(nav.x/8+0.5),(int)floor(nav.y/8+0.5));
 if(strcmp(previousRoom,label)){SetWindowTextA(roomLabel,label);strcpy(previousRoom,label);}
}
static void paint(void){
 PAINTSTRUCT ps;HDC dc=BeginPaint(wnd,&ps),draw;RECT r;BITMAPINFO bi;int fw=nav.skin?434:SNAV_W,fh=nav.skin?509:SNAV_H;
 int top=(fullscreen||screenOnly)?0:66,bottom=(fullscreen||screenOnly)?0:78;double scale;int dw,dh;
 GetClientRect(wnd,&r);
 if(!prepare_buffer(dc,r.right,r.bottom)){EndPaint(wnd,&ps);return;}draw=backDC;
 FillRect(draw,&r,(HBRUSH)GetStockObject(BLACK_BRUSH));
 scale=fmin((double)r.right/fw,(double)(r.bottom-top-bottom)/fh);if(scale<0.1)scale=0.1;
 dw=(int)(fw*scale);dh=(int)(fh*scale);displayRect.left=(r.right-dw)/2;displayRect.top=top+(r.bottom-top-bottom-dh)/2;
 displayRect.right=displayRect.left+dw;displayRect.bottom=displayRect.top+dh;
 nav_render(&nav,frame,nav.skin);memset(&bi,0,sizeof(bi));bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
 bi.bmiHeader.biWidth=fw;bi.bmiHeader.biHeight=-fh;bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;bi.bmiHeader.biCompression=BI_RGB;
 SetStretchBltMode(draw,COLORONCOLOR);StretchDIBits(draw,displayRect.left,displayRect.top,dw,dh,0,0,fw,fh,frame,&bi,DIB_RGB_COLORS,SRCCOPY);
 BitBlt(dc,0,0,r.right,r.bottom,draw,0,0,SRCCOPY);
 EndPaint(wnd,&ps);
}
static int device_button(int sx,int sy){
 int x,y;if(!nav.skin||displayRect.right<=displayRect.left)return 0;
 x=(sx-displayRect.left)*434/(displayRect.right-displayRect.left);y=(sy-displayRect.top)*509/(displayRect.bottom-displayRect.top);
 if(abs(x-216)<22&&abs(y-369)<18)return VK_UP;
 if(abs(x-216)<22&&abs(y-423)<18)return VK_DOWN;
 if(abs(x-188)<18&&abs(y-395)<22)return VK_LEFT;
 if(abs(x-246)<18&&abs(y-395)<22)return VK_RIGHT;
 if(abs(x-289)<23&&abs(y-367)<20)return VK_RETURN;
 if(abs(x-140)<22&&abs(y-420)<22)return 'P';
 if(abs(x-143)<20&&abs(y-368)<18)return VK_TAB;
 if(abs(x-289)<24&&abs(y-425)<20)return 'R';
 return 0;
}
static LRESULT CALLBACK proc(HWND h,UINT m,WPARAM w,LPARAM l){
 switch(m){
 case WM_SIZE:layout_controls();return 0;
 case WM_KILLFOCUS:clear_keys();return 0;
 case WM_ACTIVATEAPP:if(!w)clear_keys();return 0;
 case WM_ERASEBKGND:return 1;
 case WM_CTLCOLORSTATIC:SetTextColor((HDC)w,RGB(210,210,210));SetBkColor((HDC)w,RGB(0,0,0));return (LRESULT)GetStockObject(BLACK_BRUSH);
 case WM_COMMAND:
  if(LOWORD(w)==ID_GEN){generate(0);return 0;}
  if(LOWORD(w)==ID_RANDOM){generate(1);return 0;}
  if(LOWORD(w)==ID_MODE&&HIWORD(w)==CBN_SELCHANGE){nav.mode=(int)SendMessageA(modeBox,CB_GETCURSEL,0,0);config_save();SetFocus(h);return 0;}
  if(LOWORD(w)>=200)command(LOWORD(w));
  return 0;
 case WM_KEYDOWN:
  if(w<256)held[w]=1;
  if(l&(1L<<30))return 0;
  switch(w){
   case VK_RETURN:nav_trigger096(&nav);break;case VK_TAB:command(ID_PAUSE);break;
   case 'D':command(ID_SKIN);break;case 'M':command(ID_MONO);break;case 'B':command(ID_BATTERY);break;
   case 'P':nav.power=!nav.power;break;case 'R':generate(0);break;case 'N':generate(1);break;
   case VK_F11:toggle_fullscreen();break;case VK_ESCAPE:if(fullscreen)toggle_fullscreen();else if(screenOnly)DestroyWindow(h);break;
   case VK_F1:command(ID_HELP);break;
  }return 0;
 case WM_KEYUP:if(w<256)held[w]=0;return 0;
 case WM_LBUTTONDOWN:{int k=device_button((short)LOWORD(l),(short)HIWORD(l));SetFocus(h);
  if(k==VK_RETURN)nav_trigger096(&nav);else if(k=='P')nav.power=!nav.power;else if(k==VK_TAB)command(ID_PAUSE);else if(k=='R')generate(0);
  else if(k){mouseKey=k;held[k]=1;SetCapture(h);}
  return 0;}
 case WM_LBUTTONUP:if(mouseKey)held[mouseKey]=0;mouseKey=0;ReleaseCapture();return 0;
 case WM_CAPTURECHANGED:if(mouseKey)held[mouseKey]=0;mouseKey=0;return 0;
 case WM_TIMER:{
  DWORD now=GetTickCount();double dt=(now-lastTime)/1000.0;NavInput in;
  lastTime=now;if(dt>0.1)dt=0.1;accumulator+=dt;
  memset(&in,0,sizeof(in));in.forward=held[VK_UP];in.back=held[VK_DOWN];in.left=held[VK_LEFT];in.right=held[VK_RIGHT];in.run=held[VK_SHIFT];in.blink=held[VK_SPACE];
  while(accumulator>=1.0/60){nav_step(&nav,in,1.0/60);accumulator-=1.0/60;}
  update_status();InvalidateRect(h,NULL,FALSE);return 0;}
 case WM_PAINT:paint();return 0;
 case WM_SETCURSOR:if(screenOnly&&LOWORD(l)==HTCLIENT){SetCursor(NULL);return TRUE;}break;
 case WM_GETMINMAXINFO:((MINMAXINFO*)l)->ptMinTrackSize.x=screenOnly?160:480;((MINMAXINFO*)l)->ptMinTrackSize.y=screenOnly?160:430;return 0;
 case WM_DESTROY:KillTimer(h,1);drop_buffer();PostQuitMessage(0);return 0;
 }return DefWindowProcA(h,m,w,l);
}
int WINAPI WinMain(HINSTANCE instance,HINSTANCE prev,LPSTR cmd,int show){
 WNDCLASSA wc;MSG msg;HMENU menu,sim,view;HFONT font=(HFONT)GetStockObject(DEFAULT_GUI_FONT);int i;HWND controls[6];
 (void)prev;(void)cmd;config_init();memset(&wc,0,sizeof(wc));wc.lpfnWndProc=proc;wc.hInstance=instance;wc.lpszClassName="InwardSNav";wc.hCursor=LoadCursor(NULL,IDC_ARROW);wc.hIcon=LoadIconA(instance,MAKEINTRESOURCEA(101));
 RegisterClassA(&wc);menu=CreateMenu();sim=CreatePopupMenu();view=CreatePopupMenu();
 AppendMenuA(sim,MF_STRING,ID_PAUSE,"Pause / resume\tTab");AppendMenuA(sim,MF_STRING,ID_RESET,"Restart seed\tR");
 AppendMenuA(sim,MF_STRING,ID_BATTERY,"Fresh battery\tB");AppendMenuA(sim,MF_SEPARATOR,0,NULL);
 AppendMenuA(sim,MF_STRING,ID_LOAD,"Import room grid...");AppendMenuA(sim,MF_STRING,ID_SAVE,"Export room grid...");
 AppendMenuA(view,MF_STRING,ID_SKIN,"Device / LCD only\tD");AppendMenuA(view,MF_STRING,ID_MONO,"Monochrome / red contacts\tM");AppendMenuA(view,MF_STRING,ID_FULLSCREEN,"Full screen\tF11");
 AppendMenuA(menu,MF_POPUP,(UINT_PTR)sim,"Simulation");AppendMenuA(menu,MF_POPUP,(UINT_PTR)view,"Display");AppendMenuA(menu,MF_STRING,ID_HELP,"Help / controls");
 wnd=CreateWindowA("InwardSNav",screenOnly?"S-NAV Screen":"S-NAV Simulator - Inward | prototype 0.2.1",(screenOnly?(WS_POPUP|WS_THICKFRAME):WS_OVERLAPPEDWINDOW)|WS_CLIPCHILDREN,CW_USEDEFAULT,CW_USEDEFAULT,screenOnly?582:760,screenOnly?536:800,NULL,screenOnly?NULL:menu,instance,NULL);
 if(!wnd)return 1;
 SetPropA(wnd,"NAVMENU",screenOnly?NULL:menu);
 SendMessageA(wnd,WM_SETICON,ICON_BIG,(LPARAM)LoadImageA(instance,MAKEINTRESOURCEA(101),IMAGE_ICON,GetSystemMetrics(SM_CXICON),GetSystemMetrics(SM_CYICON),LR_SHARED));
 SendMessageA(wnd,WM_SETICON,ICON_SMALL,(LPARAM)LoadImageA(instance,MAKEINTRESOURCEA(101),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),LR_SHARED));
 if(!screenOnly){
 seedBox=CreateWindowA("EDIT",configuredSeed,WS_CHILD|WS_VISIBLE|WS_BORDER|ES_AUTOHSCROLL,0,0,1,1,wnd,(HMENU)ID_SEED,instance,NULL);SendMessageA(seedBox,EM_SETLIMITTEXT,95,0);
 genButton=CreateWindowA("BUTTON","Generate",WS_CHILD|WS_VISIBLE,0,0,1,1,wnd,(HMENU)ID_GEN,instance,NULL);
 randomButton=CreateWindowA("BUTTON","Random",WS_CHILD|WS_VISIBLE,0,0,1,1,wnd,(HMENU)ID_RANDOM,instance,NULL);
 modeBox=CreateWindowA("COMBOBOX","",WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST,0,0,1,1,wnd,(HMENU)ID_MODE,instance,NULL);
 for(i=0;i<4;i++)SendMessageA(modeBox,CB_ADDSTRING,0,(LPARAM)modes[i]);
 SendMessageA(modeBox,CB_SETCURSEL,nav.mode,0);
 status=CreateWindowA("STATIC","",WS_CHILD|WS_VISIBLE,0,0,1,1,wnd,NULL,instance,NULL);
 roomLabel=CreateWindowA("STATIC","",WS_CHILD|WS_VISIBLE,0,0,1,1,wnd,NULL,instance,NULL);
 controls[5]=roomLabel;controls[0]=seedBox;controls[1]=genButton;controls[2]=randomButton;controls[3]=modeBox;controls[4]=status;
 for(i=0;i<6;i++)SendMessageA(controls[i],WM_SETFONT,(WPARAM)font,TRUE);
 }
 layout_controls();update_status();lastTime=GetTickCount();SetTimer(wnd,1,16,NULL);ShowWindow(wnd,show);if(configuredFullscreen)toggle_fullscreen();SetFocus(wnd);
 while(GetMessageA(&msg,NULL,0,0)>0){TranslateMessage(&msg);DispatchMessageA(&msg);}return (int)msg.wParam;
}
