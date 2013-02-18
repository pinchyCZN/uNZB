#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#include "resource.h"
#include "Commctrl.h"
HWND hlistview=0;
char last_path[MAX_PATH]={0};

typedef struct {
	char *name;
	int width;
	int index;
}COLUMN;

COLUMN columns[]={
	{"filename",180,0},
	{"download path",100,1},
	{"status",80,2},
	{"group",70,3},
	{"poster",70,4},
	{"date",60,5},
	{"subject",60,6},
};
/*
COLUMN columns[]={
	{"filename",180,0},
	{"download path",100,1},
	{"status",80,2},
	{"group",70,3},
	{"poster",70,4},
	{"date",60,5},
	{"subject",60,6},
};
*/
COLUMN def_cols[sizeof(columns)/sizeof(COLUMN)];

int create_default_col_list(COLUMN cols[],int count)
{
	int i;
	for(i=0;i<sizeof(columns)/sizeof(COLUMN);i++){
		if(i>count)
			break;
		def_cols[i]=cols[i];
	}
	return TRUE;
}
int check_column_order()
{
	int i;
	char p[sizeof(columns)/sizeof(COLUMN)];
	memset(p,0,sizeof(p));
	columns[0].index=0;//filename always first
	p[0]++;
	for(i=1;i<sizeof(columns)/sizeof(COLUMN);i++){
		if(columns[i].index<(sizeof(columns)/sizeof(COLUMN)))
			p[columns[i].index]++;
	}
	for(i=0;i<sizeof(columns)/sizeof(COLUMN);i++){
		int j;
		if(p[i]!=1){
			for(j=0;j<sizeof(columns)/sizeof(COLUMN);j++){
				columns[j].index=def_cols[j].index;
			}
			break;
		}
	}
	return TRUE;
}
int load_ini_listview(HWND hlistview)
{
	int i;
	char key[255];
	LV_COLUMN col;
	for(i=0;i<sizeof(columns)/sizeof(COLUMN);i++){
		sprintf(key,"column_width_%s",columns[i].name);
		get_ini_value(key,&columns[i].width);
	}
	for(i=0;i<sizeof(columns)/sizeof(COLUMN);i++){
		sprintf(key,"column_index_%s",columns[i].name);
		get_ini_value(key,&columns[i].index);
	}
	check_column_order();
	for(i=0;i<sizeof(columns)/sizeof(COLUMN);i++){
		int j;
		for(j=0;j<sizeof(columns)/sizeof(COLUMN);j++){
			if(i==columns[j].index){
				col.mask = LVCF_WIDTH|LVCF_TEXT;
				col.cx = columns[j].width;
				col.pszText = columns[j].name;
				ListView_InsertColumn(hlistview,columns[j].index,&col);
				break;
			}
		}
	}
	return 0;
}
int save_list_ini()
{
	int i;
	char key[255],val[255];
	for(i=0;i<sizeof(columns)/sizeof(COLUMN);i++){
		int width,index;
		if(lv_info_by_name(columns[i].name,&width,&index)){
			columns[i].index=index;
			columns[i].width=width;
		}
		else{
			columns[i].index=def_cols[i].index;
			columns[i].width=def_cols[i].width;
		}
	}
	for(i=0;i<sizeof(columns)/sizeof(COLUMN);i++){
		sprintf(key,"column_width_%s",columns[i].name);
		sprintf(val,"%i",columns[i].width);
		write_ini_str(key,val);
	}
	for(i=0;i<sizeof(columns)/sizeof(COLUMN);i++){
		sprintf(key,"column_index_%s",columns[i].name);
		sprintf(val,"%i",columns[i].index);
		write_ini_str(key,val);
	}
	return 0;
}
int write_new_list_ini()
{
	create_default_col_list(columns,sizeof(columns)/sizeof(COLUMN));
	save_list_ini();
	write_ini_str("threads","4");
	return TRUE;
}

HWND CreateListView(HWND hwnd) 
{
    INITCOMMONCONTROLSEX ctrls;
    RECT rclient;

	ctrls.dwSize=sizeof(ctrls);
    ctrls.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&ctrls);

    GetClientRect (hwnd, &rclient); 

    hlistview = CreateWindow(WC_LISTVIEW, 
                                     "",
                                     WS_TABSTOP|WS_CHILD|WS_VISIBLE|LVS_REPORT|LVS_SHOWSELALWAYS, //|LVS_OWNERDRAWFIXED, //|LVS_EDITLABELS,
                                     0, 32,
                                     rclient.right - rclient.left,
                                     rclient.bottom - rclient.top-60,
                                     hwnd,
                                     2000, //handle ID
                                     (HINSTANCE)GetWindowLong(hwnd,GWL_HINSTANCE),
                                     NULL);
	ListView_SetExtendedListViewStyle(hlistview,
		ListView_GetExtendedListViewStyle(hlistview)|LVS_EX_FULLROWSELECT);
	create_default_col_list(columns,sizeof(columns)/sizeof(COLUMN));
	load_ini_listview(hlistview);
	subclass_listview();
	create_popup_menus();
    return hlistview;
}

HMENU item_menu=0,column_menu=0;
enum {
	CMD_DLFOLDER1=1100,
	CMD_SETDLFOLDER=1200,
	CMD_SETLASTPATH,
	CMD_RESETSTATUS,
	CMD_RESETINCOMPLETE,
	CMD_REMOVEITEMS,
	CMD_MOVETOTOP,
	CMD_SELECTALL,
	CMD_COLUMNS=1300
};
int get_max_dirs()
{
	return 9;
}
int create_popup_menus()
{
	int i;
	if(item_menu!=0)DestroyMenu(item_menu);
	if(item_menu=CreatePopupMenu()){
		for(i=0;i<get_max_dirs();i++){
			char key[80];
			char str[MAX_PATH+100];
			str[0]=0;
			_snprintf(key,sizeof(key),"default_dir%i",i+1);
			get_ini_str(key,str,sizeof(str));
			if(str[0]!=0){
				InsertMenu(item_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_DLFOLDER1+i,str);
			}
		}
		InsertMenu(item_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_SETLASTPATH,"[last browsed path used]");
		InsertMenu(item_menu,0xFFFFFFFF,MF_BYPOSITION|MF_SEPARATOR,0,0);
		InsertMenu(item_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_SETDLFOLDER,"set download folder");
		InsertMenu(item_menu,0xFFFFFFFF,MF_BYPOSITION|MF_SEPARATOR,0,0);
		InsertMenu(item_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_RESETSTATUS,"reset status all");
		InsertMenu(item_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_RESETINCOMPLETE,"reset status incomplete");
		InsertMenu(item_menu,0xFFFFFFFF,MF_BYPOSITION|MF_SEPARATOR,0,0);
//		InsertMenu(item_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_MOVETOTOP,"move items to top");
		InsertMenu(item_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_SELECTALL,"select all items");
		InsertMenu(item_menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,CMD_REMOVEITEMS,"remove items");
	}
	if(column_menu!=0)DestroyMenu(column_menu);
	if(column_menu=CreatePopupMenu()){
		int flags=MF_BYPOSITION|MF_STRING;
		for(i=0;i<sizeof(columns)/sizeof(COLUMN);i++)
			InsertMenu(column_menu,0xFFFFFFFF,flags,CMD_COLUMNS+i,columns[i].name);
		InsertMenu(column_menu,0xFFFFFFFF,MF_BYPOSITION|MF_SEPARATOR,0,0);
		InsertMenu(column_menu,0xFFFFFFFF,flags,CMD_COLUMNS+i,"reset");
	}
	return 0;
}
int toggle_col_width(char *name)
{
	int width,index;
	if(lv_info_by_name(name,&width,&index)){
		if(width>5)
			ListView_SetColumnWidth(hlistview,index,0);
		else
			ListView_SetColumnWidth(hlistview,index,def_cols[index].width);
	}
	else{
		int i;
		for(i=0;i<sizeof(columns)/sizeof(COLUMN);i++){
			ListView_SetColumnWidth(hlistview,i,def_cols[i].width);
		}
	}
	return TRUE;
}
int lv_info_by_name(char *name,int *width,int *index)
{
	int i;
	for(i=0;i<sizeof(columns)/sizeof(COLUMN);i++){
		char str[255];
		LV_COLUMN col;
		str[0]=0;
		col.mask=LVCF_TEXT|LVCF_WIDTH;
		col.pszText=str;
		col.cchTextMax=sizeof(str);
		ListView_GetColumn(hlistview,i,&col);
		if(strcmp(str,name)==0){
			*width=col.cx;
			*index=i;
			return TRUE;
		}
	}
	return FALSE;
}
int get_column_index(char *name,int *index)
{
	int i;
	for(i=0;i<sizeof(columns)/sizeof(COLUMN);i++){
		if(strcmp(columns[i].name,name)==0){
				*index=columns[i].index;
				return TRUE;
		}
	}
	return FALSE;
}
int go_up_one_level()
{
	char path[MAX_PATH];
	int i,count,index=1; //default
	get_column_index("download path",&index);
	count=ListView_GetItemCount(hlistview);
	for(i=0;i<count;i++){
		if(ListView_GetItemState(hlistview,i,LVIS_SELECTED)){
			path[0]=0;
			ListView_GetItemText(hlistview,i,index,path,sizeof(path));
			if(path[0]!=0){
				int j,len=strlen(path);
				if(len>3 && path[len-1]=='\\')
					path[len-1]=0;
				for(j=len-1;j>2;j--){
					if(path[j]!='\\')
						path[j]=0;
					else
						break;
				}
				ListView_SetItemText(hlistview,i,index,path);
			}
		}
	}
	return TRUE;
}
int set_dl_folder(HWND hwnd)
{
	char path[MAX_PATH];
	int i,count=0,selected=0;
	int index=1; //default
	path[0]=0;
	get_column_index("download path",&index);
	count=ListView_GetItemCount(hlistview);
	for(i=0;i<count;i++){
		if(ListView_GetItemState(hlistview,i,LVIS_SELECTED)){
			selected++;
			path[0]=0;
			ListView_GetItemText(hlistview,i,index,path,sizeof(path));
			if(path[0]!=0 && find_valid_path(path))
				break;
		}
	}
	if(selected==0)
		return FALSE;
	if(handle_browse_dialog(hwnd,path)){
		strncpy(last_path,path,sizeof(last_path));
		for(i=0;i<count;i++){
			if(ListView_GetItemState(hlistview,i,LVIS_SELECTED)){
				ListView_SetItemText(hlistview,i,index,path);
			}
		}
		write_ini_str("last_dir",last_path);
		return TRUE;
	}
	return FALSE;
}
int add_item(const char *fname,
			 const char *dlpath,
			 const char *status,
			 const char *group,
			 const char *poster,
			 const char *date,
			 const char *subject,
			 LPVOID xml
			 )
{
	LVITEM listItem;
	int index=0,item=0x80000000-1; //add to end of list
	listItem.mask = LVIF_TEXT;
	listItem.pszText = fname;
	listItem.iItem = item;
	listItem.iSubItem =0;
	item=ListView_InsertItem(hlistview,&listItem);
	if(item>=0){
		if(get_column_index("download path",&index)){
			ListView_SetItemText(hlistview,item,index,dlpath);
		}
		if(get_column_index("status",&index)){
			ListView_SetItemText(hlistview,item,index,status);
		}
		if(get_column_index("group",&index)){
			ListView_SetItemText(hlistview,item,index,group);
		}
		if(get_column_index("poster",&index)){
			ListView_SetItemText(hlistview,item,index,poster);
		}
		if(get_column_index("date",&index)){
			ListView_SetItemText(hlistview,item,index,date);
		}
		if(get_column_index("subject",&index)){
			ListView_SetItemText(hlistview,item,index,subject);
		}
		listItem.mask=LVIF_PARAM;
		listItem.lParam=xml;
		listItem.iItem=item;
		ListView_SetItem(hlistview,&listItem);
		return TRUE;
	}
	return FALSE;
}
WNDPROC wporiglistview=0;
LRESULT APIENTRY sc_listview(
    HWND hwnd, 
    UINT msg, 
    WPARAM wparam, 
    LPARAM lparam) 
{
	static DWORD tick;
	int xpos,ypos;
	int flags,item;
#ifdef _DEBUG
	if(FALSE)
	if(msg!=WM_GETDLGCODE&&/*msg!=WM_KEYFIRST&&*/msg!=WM_MOUSEFIRST&&msg!=WM_NCHITTEST&&msg!=WM_SETCURSOR&&msg!=WM_ENTERIDLE&&msg!=WM_NOTIFY)
	{
		if((GetTickCount()-tick)>500)
			printf("--\n");
		tick=GetTickCount();	
		print_msg(msg,lparam,wparam);
	}
#endif
	switch(msg){
	case WM_COMMAND:
		switch(LOWORD(wparam)){
		case WM_DESTROY:
			break;
		case CMD_SETDLFOLDER:
			set_dl_folder(hwnd);
			break;
		case CMD_SETLASTPATH:
			if(last_path[0]!=0)
				update_selected_paths(last_path);
			break;
		case CMD_RESETSTATUS:
			reset_selected_items();
			break;
		case CMD_RESETINCOMPLETE:
			reset_incomplete_items();
			break;
		case CMD_REMOVEITEMS:
			delete_items();
			update_byte_counters();
			break;
		case CMD_MOVETOTOP:
			break;
		case CMD_SELECTALL:
			select_all();
			break;
		default:
			if(LOWORD(wparam)>=CMD_COLUMNS){
				MENUITEMINFO mii;
				char str[80];
				str[0]=0;
				mii.cbSize=sizeof(mii);
				mii.fMask=MIIM_TYPE;
				mii.fState=0;
				mii.fType=MFT_STRING;
				mii.dwTypeData=str;
				mii.cch=sizeof(str);
				GetMenuItemInfo(column_menu,LOWORD(wparam),FALSE,&mii);
				toggle_col_width(str);
				save_list_ini();
				//printf("%08X %08X\n",
			}
			else if(LOWORD(wparam)>=CMD_DLFOLDER1){
				char path[MAX_PATH];
				path[0]=0;
				GetMenuString(item_menu,LOWORD(wparam),path,sizeof(path),MF_BYCOMMAND);
				if(path[0]!=0)
					update_selected_paths(path);
			}
			break;
		}
		break;
	case WM_CONTEXTMENU:
		xpos=LOWORD(lparam);
		ypos=HIWORD(lparam);
		if(ypos<=70)
			TrackPopupMenu(column_menu,TPM_LEFTALIGN,xpos,ypos,0,hlistview,NULL);
		else if(ypos>70 && ListView_GetSelectedCount(hlistview)>0){
			if(last_path[0]!=0){
				MENUITEMINFO mii;
				mii.cbSize=sizeof(mii);
				mii.fMask=MIIM_TYPE;
				mii.fType=MFT_STRING;
				mii.dwTypeData=last_path;
				SetMenuItemInfo(item_menu,CMD_SETLASTPATH,FALSE,&mii);
			}
			TrackPopupMenu(item_menu,TPM_LEFTALIGN,xpos,ypos,0,hlistview,NULL);
		}
		break;
	case WM_MENUSELECT:
		flags=HIWORD(wparam);
		item=LOWORD(wparam);
		if(flags&MF_MOUSESELECT){
		//	printf("item %i selection\n",item);
		}
	case WM_CHAR:
		switch(LOWORD(wparam)){
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		}
		return 0;
		break;
	case WM_KEYFIRST:
		break;
	case WM_GETDLGCODE:
		break;
		return DLGC_WANTALLKEYS;
	case WM_CLOSE:
	case WM_QUIT:
		break;
	}
 
    return CallWindowProc(wporiglistview, hwnd, msg, 
        wparam, lparam); 
} 



int subclass_listview()
{
	wporiglistview=SetWindowLong(hlistview,GWL_WNDPROC,(LONG)sc_listview);
	return TRUE;
}

int update_selected_paths(char *path)
{
	int i,count;
	count=ListView_GetItemCount(hlistview);
	for(i=count-1;i>=0;i--){
		if(ListView_GetItemState(hlistview,i,LVIS_SELECTED)){
			ListView_SetItemText(hlistview,i,1,path);
		}
	}
	return TRUE;
}
int reset_selected_items()
{
	int i,count;
	count=ListView_GetItemCount(hlistview);
	for(i=count-1;i>=0;i--){
		LV_ITEM lv;
		lv.mask=LVIF_PARAM|LVIF_STATE;
		lv.lParam=0;
		lv.iItem=i;
		lv.iSubItem=0;
		lv.state=0;
		lv.stateMask=LVIS_SELECTED;
		if(ListView_GetItem(hlistview,&lv)){
			if(lv.state&LVIS_SELECTED){
				int segments=0,xml=lv.lParam;
				set_segment_status(xml,0,FALSE,0,TRUE);
				segments=get_segment_count(xml);
				remove_all_thread_id(xml);
				set_item_status_by_xml(xml,"[0/%i]",segments);
			}
		}
	}
	return TRUE;
}
int reset_incomplete_items()
{
	int i,count;
	count=ListView_GetItemCount(hlistview);
	for(i=count-1;i>=0;i--){
		LV_ITEM lv;
		lv.mask=LVIF_PARAM|LVIF_STATE;
		lv.lParam=0;
		lv.iItem=i;
		lv.iSubItem=0;
		lv.state=0;
		lv.stateMask=LVIS_SELECTED;
		if(ListView_GetItem(hlistview,&lv)){
			if(lv.state&LVIS_SELECTED){
				int downloaded,segments=0,xml=lv.lParam;
				unset_incomplete_status(xml);
				segments=get_segment_count(xml);
				downloaded=get_segments_downloaded(xml);
				remove_all_thread_id(xml);
				set_item_status_by_xml(xml,"[%i/%i]%s",downloaded,segments,
					downloaded>=segments?" done":"");
			}
		}
	}
	return TRUE;
}
int delete_items()
{
	int i,count;
	count=ListView_GetItemCount(hlistview);
	for(i=count-1;i>=0;i--){
		LV_ITEM lv;
		lv.mask=LVIF_PARAM|LVIF_STATE;
		lv.lParam=0;
		lv.iItem=i;
		lv.iSubItem=0;
		lv.state=0;
		lv.stateMask=LVIS_SELECTED;
		if(ListView_GetItem(hlistview,&lv)){
			if(lv.state&LVIS_SELECTED){
				free_xml_node(lv.lParam);
				ListView_DeleteItem(hlistview,i);
			}
		}
	}
	return TRUE;
}
int select_all()
{
	int i,count;
	count=ListView_GetItemCount(hlistview);
	for(i=0;i<count;i++)
		ListView_SetItemState(hlistview,i,LVIS_SELECTED,LVIS_SELECTED);
	return 0;
}
int items_selected()
{
	return ListView_GetSelectedCount(hlistview);
}
int get_xml_param(int item)
{
	LV_ITEM lv;
	lv.mask=LVIF_PARAM;
	lv.lParam=0;
	lv.iItem=item;
	lv.iSubItem=0;
	if(ListView_GetItem(hlistview,&lv)){
		return lv.lParam;
	}
	return 0;
}
int find_item_by_xml(int xml)
{
	LV_FINDINFO lvf;
	lvf.flags=LVFI_PARAM;
	lvf.lParam=xml;
	return ListView_FindItem(hlistview,-1,&lvf);
}
int set_item_status_by_xml(int xml,char *fmt,...)
{
	va_list args;
	char str[256];
	int index=0;
	memset(str,0,sizeof(str));
	va_start(args,fmt);
	_vsnprintf(str,sizeof(str)-1,fmt,args);
	va_end(args);
	if(get_column_index("status",&index)){
		int item=find_item_by_xml(xml);
		if(item>=0){
			ListView_SetItemText(hlistview,item,index,str);
			return TRUE;
		}
	}
	return FALSE;
}

int get_item_dl_path(int xml,char *str,int len)
{
	int index,item=find_item_by_xml(xml);
	if(item>=0){
		if(get_column_index("download path",&index)){
			ListView_GetItemText(hlistview,item,index,str,len);
			return TRUE;
		}

	}
	return FALSE;
}
int listview_get_count()
{
	return ListView_GetItemCount(hlistview);
}

int is_all_dl_paths_set()
{
	int found=0;
	char path[MAX_PATH];
	int i,xml,items=listview_get_count();
	for(i=0;i<items;i++){
		xml=get_xml_param(i);
		if(xml!=0){
			path[0]=0;
			get_item_dl_path(xml,&path,sizeof(path));
			if(path[0]!=0)
				found++;
		}
	}
	return found==items;
}
int restore_last_path()
{
	return get_ini_str("last_dir",last_path,sizeof(last_path));
}