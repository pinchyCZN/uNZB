#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#include "resource.h"

enum{
	CONTROL_ID,
	XPOS,YPOS,
	WIDTH,HEIGHT,
	HUG_L,
	HUG_R,
	HUG_T,
	HUG_B,
	HUG_CTRL_X,
	HUG_CTRL_Y,
	HUG_HEIGHT,
	HUG_WIDTH,
	HUG_CTRL_TXT_X,
	HUG_CTRL_TXT_Y,
	HUG_CTRL_TXT_X_,
	HUG_CTRL_TXT_Y_,
	SIZE_HEIGHT_OFF,
	SIZE_WIDTH_OFF,
	SIZE_HEIGHT_PER,
	SIZE_WIDTH_PER,
	SIZE_TEXT_CTRL,
	CONTROL_FINISH,
	RESIZE_FINISH
};

int process_anchor_list(HWND hwnd,short *list)
{
	int limit=9999;
	int i=0,j,x,y,width,height;
	HWND dlg_item;
	HDC	 hdc;
	RECT crect;
	SIZE text_size;
	char str[255];
	double f;
	int done=FALSE;
	int last_text=0;

	memset(&crect,0,sizeof(crect));
	hdc=GetDC(hwnd);
	GetClientRect(hwnd, &crect);
	do{
		switch(list[i]){
		case CONTROL_ID:
			x=y=width=height=0;
			dlg_item=GetDlgItem(hwnd,list[i+1]);
			if(dlg_item==NULL)
				done=TRUE;
			break;
		case XPOS:
			x+=list[i+1];
			break;
		case YPOS:
			y+=list[i+1];
			break;
		case WIDTH:
			width+=list[i+1];
			break;
		case HEIGHT:
			height+=list[i+1];
			break;
		case HUG_L:
			x+=crect.left+list[i+1];
			break;
		case HUG_R:
			x+=crect.right+list[i+1];
			break;
		case HUG_T:
			y+=crect.top+list[i+1];
			break;
		case HUG_B:
			y+=crect.bottom+list[i+1];
			break;
		case HUG_CTRL_X:
			break;
		case HUG_CTRL_Y:
			break;
		case HUG_HEIGHT:
			j=crect.bottom-crect.top;
			f=(double)list[i+1]/1000.0;
			y+=j*f;
			break;
		case HUG_WIDTH:
			j=crect.right-crect.left;
			f=(double)list[i+1]/1000.0;
			x+=j*f;
			break;
		case HUG_CTRL_TXT_X:
			if(last_text!=list[i+1]){
				GetDlgItemText(hwnd,list[i+1],str,sizeof(str)-1);
				GetTextExtentPoint32(hdc,str,strlen(str),&text_size);
				last_text=list[i+1];
			}
			x+=text_size.cx;
			break;
		case HUG_CTRL_TXT_X_:
			if(last_text!=list[i+1]){
				GetDlgItemText(hwnd,list[i+1],str,sizeof(str)-1);
				GetTextExtentPoint32(hdc,str,strlen(str),&text_size);
				last_text=list[i+1];
			}
			x-=text_size.cx;
			break;
		case HUG_CTRL_TXT_Y:
			if(last_text!=list[i+1]){
				GetDlgItemText(hwnd,list[i+1],str,sizeof(str)-1);
				GetTextExtentPoint32(hdc,str,strlen(str),&text_size);
				last_text=list[i+1];
			}
			y+=text_size.cy;
			break;
		case HUG_CTRL_TXT_Y_:
			if(last_text!=list[i+1]){
				GetDlgItemText(hwnd,list[i+1],str,sizeof(str)-1);
				GetTextExtentPoint32(hdc,str,strlen(str),&text_size);
				last_text=list[i+1];
			}
			y-=text_size.cy;
			break;
		case SIZE_HEIGHT_OFF:
			height+=crect.bottom-crect.top+list[i+1];
			break;
		case SIZE_WIDTH_OFF:
			width+=crect.right-crect.left+list[i+1];
			break;
		case SIZE_HEIGHT_PER:
			j=crect.bottom-crect.top;
			f=(double)list[i+1]/1000.0;
			height+=f*j;
			break;
		case SIZE_WIDTH_PER:
			j=crect.right-crect.left;
			f=(double)list[i+1]/1000.0;
			width+=f*j;
			break;
		case SIZE_TEXT_CTRL:
			if(last_text!=list[i+1]){
				GetDlgItemText(hwnd,list[i+1],str,sizeof(str)-1);
				GetTextExtentPoint32(hdc,str,strlen(str),&text_size);
				last_text=list[i+1];
			}
			width+=text_size.cx;
			height+=text_size.cy;
			break;
		case CONTROL_FINISH:
			SetWindowPos(dlg_item,NULL,x,y,width,height,SWP_NOZORDER);
			break;
		case RESIZE_FINISH:
			done=TRUE;
			break;
		default:
			printf("bad command %i\n",list[i]);
			break;
		}
		i+=2;
		if(i>limit)
			done=TRUE;
	}while(!done);
	ReleaseDC(hwnd,hdc);
	return TRUE;
}
short settings_anchors[]={
	CONTROL_ID,IDC_DEFAULT_DIR,
				HUG_L,11,
				YPOS,241,
				SIZE_WIDTH_OFF,-21,HEIGHT,200,CONTROL_FINISH,-1,
	CONTROL_ID,IDC_NEWS_SERVER,
				XPOS,21,
				YPOS,30,
				SIZE_WIDTH_OFF,-121,HEIGHT,24,CONTROL_FINISH,-1,

	RESIZE_FINISH
};
short main_dlg_anchors[]={
	CONTROL_ID,2000, //hlistview
		HUG_L,0,
		SIZE_WIDTH_OFF,0,
		HUG_T,32,
		SIZE_HEIGHT_OFF,-60,
		CONTROL_FINISH,-1,
	CONTROL_ID,IDC_STATUS,
		XPOS,1,
		SIZE_WIDTH_OFF,-33,
		HUG_B,-26,
		HEIGHT,26,
		CONTROL_FINISH,-1,
	CONTROL_ID,IDCANCEL,
		WIDTH,75,
		HEIGHT,23,
		HUG_T,0,
		HUG_R,-78,
		CONTROL_FINISH,-1,
	CONTROL_ID,IDC_GONOW,
		WIDTH,75,
		HEIGHT,23,
		HUG_T,0,
		HUG_WIDTH,555,
		CONTROL_FINISH,-1,
	RESIZE_FINISH

/*
	CONTROL_ID,IDC_SLIDER1,
				HUG_L,5,
				HUG_B,-35,
				SIZE_WIDTH_OFF,-25,HEIGHT,30,CONTROL_FINISH,-1,
	CONTROL_ID,IDCANCEL,
				XPOS,5,HUG_WIDTH,870,
				HUG_T,5,
				SIZE_WIDTH_PER,125,SIZE_HEIGHT_PER,166,CONTROL_FINISH,-1,
	CONTROL_ID,ID_SYNCTIME,
				XPOS,5,HUG_WIDTH,870,
				HUG_T,10,HUG_HEIGHT,166,
				SIZE_WIDTH_PER,125,SIZE_HEIGHT_PER,166,CONTROL_FINISH,-1,
	CONTROL_ID,IDC_ONTOP1, XPOS,-95,HUG_WIDTH,870,HUG_T,5,SIZE_TEXT_CTRL,IDC_ONTOP1,CONTROL_FINISH,-1,
	CONTROL_ID,IDC_FREEZE, XPOS,-95,HUG_WIDTH,870,HUG_T,22,SIZE_TEXT_CTRL,IDC_ONTOP1,CONTROL_FINISH,-1,
	CONTROL_ID,IDC_UTC,    XPOS,-95,HUG_WIDTH,870,HUG_T,39,SIZE_TEXT_CTRL,IDC_ONTOP1,CONTROL_FINISH,-1,
	CONTROL_ID,IDC_NTPSERVER,
				HUG_R,-5,HUG_CTRL_TXT_X_,IDC_NTPSERVER,
				HUG_HEIGHT,333,YPOS,14,
				SIZE_TEXT_CTRL,IDC_NTPSERVER,CONTROL_FINISH,-1,
	CONTROL_ID,IDC_HELPSTATIC,HUG_L,15,HUG_B,-55,SIZE_TEXT_CTRL,IDC_HELPSTATIC,CONTROL_FINISH,-1,
	CONTROL_ID,IDC_STATIC_CURRENT_TIME,
				HUG_L,15,
				HUG_HEIGHT,21,
				SIZE_TEXT_CTRL,IDC_STATIC_CURRENT_TIME,CONTROL_FINISH,-1,
	CONTROL_ID,IDC_CTIME,  
				HUG_L,15,
				HUG_HEIGHT,21,HUG_CTRL_TXT_Y,IDC_STATIC_CURRENT_TIME,
				SIZE_TEXT_CTRL,IDC_CTIME,HEIGHT,10,CONTROL_FINISH,-1,
	CONTROL_ID,IDC_STATIC_TARGET_TIME,
				HUG_WIDTH,333,
				HUG_HEIGHT,21,
				SIZE_TEXT_CTRL,IDC_STATIC_TARGET_TIME,CONTROL_FINISH,-1,
	CONTROL_ID,IDC_TARGET,
				HUG_WIDTH,333,
				HUG_HEIGHT,21,HUG_CTRL_TXT_Y,IDC_STATIC_TARGET_TIME,
				SIZE_TEXT_CTRL,IDC_TARGET,HEIGHT,10,CONTROL_FINISH,-1,
	CONTROL_ID,IDC_CLRHOUR,
				HUG_WIDTH,333,
				YPOS,10,HUG_HEIGHT,21,HUG_CTRL_TXT_Y,IDC_STATIC_TARGET_TIME,HUG_CTRL_TXT_Y,IDC_TARGET,
				WIDTH,20,HEIGHT,20,SIZE_TEXT_CTRL,IDC_CLRHOUR,CONTROL_FINISH,-1,
	CONTROL_ID,IDC_CLRMIN,
				XPOS,20,HUG_WIDTH,333,HUG_CTRL_TXT_X,IDC_CLRHOUR,
				YPOS,10,HUG_HEIGHT,21,HUG_CTRL_TXT_Y,IDC_STATIC_TARGET_TIME,HUG_CTRL_TXT_Y,IDC_TARGET,
				WIDTH,20,HEIGHT,20,SIZE_TEXT_CTRL,IDC_CLRHOUR,CONTROL_FINISH,-1,
	CONTROL_ID,IDC_CLRSEC,
				XPOS,40,HUG_WIDTH,333,HUG_CTRL_TXT_X,IDC_CLRHOUR,HUG_CTRL_TXT_X,IDC_CLRHOUR,
				YPOS,10,HUG_HEIGHT,21,HUG_CTRL_TXT_Y,IDC_STATIC_TARGET_TIME,HUG_CTRL_TXT_Y,IDC_TARGET,
				WIDTH,20,HEIGHT,20,SIZE_TEXT_CTRL,IDC_CLRHOUR,CONTROL_FINISH,-1,
	CONTROL_ID,IDC_LIST1,
				XPOS,20,HUG_WIDTH,500,HUG_HEIGHT,500,SIZE_WIDTH_PER,100,SIZE_HEIGHT_PER,100,

				CONTROL_FINISH,-1,

	RESIZE_FINISH
*/
};
int reposition_controls(HWND hwnd, short *list)
{
	RECT	rect;
	GetClientRect(hwnd, &rect);
	process_anchor_list(hwnd,list);
	InvalidateRect(hwnd,&rect,TRUE);
	return TRUE;
}
#define GRIPPIE_SQUARE_SIZE 15
int create_grippy(HWND hwnd,HWND grippy)
{
	RECT client_rect;
	GetClientRect(hwnd,&client_rect);
	
	return CreateWindow("Scrollbar",NULL,WS_CHILD|WS_VISIBLE|SBS_SIZEGRIP,
		client_rect.right-GRIPPIE_SQUARE_SIZE,
		client_rect.bottom-GRIPPIE_SQUARE_SIZE,
		GRIPPIE_SQUARE_SIZE,GRIPPIE_SQUARE_SIZE,
		hwnd,NULL,NULL,NULL);
}

int grippy_move(HWND hwnd,HWND grippy)
{
	RECT client_rect;
	GetClientRect(hwnd,&client_rect);
	if(grippy!=0)
	{
		int i;
		SetWindowPos(grippy,NULL,
			client_rect.right-GRIPPIE_SQUARE_SIZE,
			client_rect.bottom-GRIPPIE_SQUARE_SIZE,
			GRIPPIE_SQUARE_SIZE,GRIPPIE_SQUARE_SIZE,
			SWP_NOZORDER|SWP_SHOWWINDOW);
		i=GetLastError();
	}
	return 0;
}