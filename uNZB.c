#define _WIN32_WINNT 0x400
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <shlwapi.h>
#include <Shlobj.h>
#include "resource.h"
#include "Commctrl.h"

#define MAX_THREADS 99
int screen_updated=TRUE;
HWND	hwindow=0;
HINSTANCE	ghinstance=0;
HANDLE event=0;
void debug_printf(char *fmt,...)
{
	va_list ap;
	char s[255];
	va_start(ap,fmt);
	_vsnprintf(s,sizeof(s),fmt,ap);
	OutputDebugString(s);
}
int move_console()
{
	BYTE Title[200]; 
	HANDLE hConWnd; 
	GetConsoleTitle(Title,sizeof(Title));
	hConWnd=FindWindow(NULL,Title);
	SetWindowPos(hConWnd,0,650,0,0,0,SWP_NOSIZE|SWP_NOZORDER);
	return 0;
}
void open_console()
{
	BYTE Title[200]; 
	BYTE ClassName[200]; 
	LPTSTR  lpClassName=ClassName; 
	HANDLE hConWnd; 
	FILE *hf;
	static BYTE consolecreated=FALSE;
	static int hCrt=0;
	
	if(consolecreated==TRUE)
	{
		GetConsoleTitle(Title,sizeof(Title));
		hConWnd=FindWindow(NULL,Title);
		GetClassName(hConWnd,lpClassName,120);
		ShowWindow(hConWnd,SW_SHOW);
		SetForegroundWindow(hConWnd);
		hConWnd=GetStdHandle(STD_INPUT_HANDLE);
		FlushConsoleInputBuffer(hConWnd);
		return;
	}
	AllocConsole(); 
	hCrt=_open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE),_O_TEXT);

	fflush(stdin);
	hf=_fdopen(hCrt,"w"); 
	*stdout=*hf; 
	setvbuf(stdout,NULL,_IONBF,0);

	GetConsoleTitle(Title,sizeof(Title));
	hConWnd=FindWindow(NULL,Title);
	GetClassName(hConWnd,lpClassName,120);
	ShowWindow(hConWnd,SW_SHOW); 
	SetForegroundWindow(hConWnd);
	consolecreated=TRUE;
}
UINT CALLBACK OFNHookProc(HWND hDlg,UINT Msg,WPARAM wParam,LPARAM lParam)
{
	
	HWND hWnd;
	RECT rect;
	static int init_size=TRUE,init_details=TRUE;
	static int scroll_pos=0;
	static int last_selection=0;
	
	switch(Msg)
	{
	case WM_INITDIALOG:
		init_details=TRUE;
		SetForegroundWindow(hDlg);
		return 0; //0=dialog process msg,nonzero=ignore
	case WM_NOTIFY:
		PostMessage(hDlg,WM_APP + 1,0,0); 
		return 0;
	case WM_APP + 1:
		{ 
			int i;
			HWND const dlg     =GetParent(hDlg); 
			HWND defView =GetDlgItem(dlg,0x0461); 
			HWND list=GetDlgItem(defView,1);
			if(init_details)
			{
				SendMessage(defView,WM_COMMAND,28716,0); //details view
				ListView_EnsureVisible(list,last_selection,FALSE);
				ListView_SetItemState(list,last_selection,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
				SetFocus(list);
				init_details=FALSE;
			}
			if(ListView_GetItemCount(list)>0)
			{
				ListView_SetColumnWidth(list,0,LVSCW_AUTOSIZE);
				ListView_SetColumnWidth(list,1,LVSCW_AUTOSIZE);
				ListView_SetColumnWidth(list,2,LVSCW_AUTOSIZE);
				ListView_SetColumnWidth(list,3,LVSCW_AUTOSIZE);
			}
			i=ListView_GetNextItem(list,-1,LVNI_SELECTED);
			if(i>=0)
				last_selection=i;
			
			hWnd=GetDesktopWindow();
			if(init_size && GetWindowRect(hWnd,&rect)) //only do at start ,later resizing operations remain
			{
				SetWindowPos(dlg,HWND_TOP,0,0,(int)(rect.right*.75),(int)(rect.bottom*.75),0);
				init_size=FALSE;
			}
		} 
		return TRUE;
	case WM_DESTROY:
		return 0;
		break;
	default:  
		return 0;
	}
	
}

int open_file(HWND hwnd,char *title,char **fnames,int *num_files)
{
	static TCHAR filter[]=TEXT("NZB files\0*.nzb;\0all files\0*.*;\0\0");
	static TCHAR tmpname[MAX_PATH*100],startpath[MAX_PATH];
	static OPENFILENAME ofn;
	memset(&ofn,0,sizeof(OPENFILENAME));
	memset(tmpname,0,sizeof(tmpname));
	
	ofn.hwndOwner		=hwnd;
	ofn.lStructSize     =sizeof(OPENFILENAME);
	ofn.lpstrFilter     =filter;
	ofn.lpstrFile       =tmpname;
	ofn.nMaxFile        =sizeof(tmpname);
	ofn.lpfnHook		=OFNHookProc;
	ofn.Flags			=OFN_ENABLEHOOK|OFN_EXPLORER|OFN_ENABLESIZING|OFN_ALLOWMULTISELECT;
	ofn.lpstrTitle		=title;
	
	if(get_ini_str("last_nzb_location",startpath,sizeof(startpath)))
		ofn.lpstrInitialDir=startpath;

	if(GetOpenFileName(&ofn)==0)
		return 0;
	else{
		*num_files=get_file_count(ofn.lpstrFile,sizeof(tmpname));
		*fnames=tmpname;
	}
	return TRUE;
}



int get_file_count(char *list,int size)
{
	int i,count=0;
	if((size>0) && list[0]==0)
		return 0;
	for(i=0;i<size;i++){
		if((i>0) && (list[i]==0)){
			count++;
			if((i<(size-1)) && list[i+1]==0)
				break;
		}
	}
	return count;
}
int get_file_name(char *list,int index,char **name)
{
	int i,count=0;
	if(list[0]==0)
		return FALSE;
	for(i=0;i<0x100000;i++){
		if(count==index)
			break;
		if((i>0) && (list[i]==0)){
			count++;
			if(list[i+1]==0)
				break;
		}
	}
	if(list[i]!=0){
		*name=list+i;
		return TRUE;
	}
	else
		return FALSE;
}
int handle_file_dialog(HWND hwnd)
{
	int i;
	char *flist,fname[MAX_PATH];
	int num_files=0;
	if(open_file(hwnd,"Select NZB file(s)",&flist,&num_files)){
		if(num_files>1){
			char *path=0;
			if(get_file_name(flist,0,&path)){
				for(i=1;i<num_files;i++){
					char *n=0;
					if(get_file_name(flist,i,&n)){
						_snprintf(fname,sizeof(fname),"%s\\%s",path,n);
						printf("file=%s\n",fname);
						load_nzb_file(fname);
					}
				}
			}
		}
		else
			load_nzb_file(flist);
		write_ini_str("last_nzb_location",flist);
	}
	return TRUE;
}
int is_path_directory(char *path)
{
	int attrib;
	attrib=GetFileAttributes(path);
	if((attrib!=0xFFFFFFFF) && (attrib&FILE_ATTRIBUTE_DIRECTORY))
		return TRUE;
	else
		return FALSE;
}
int find_valid_path(char *path)
{
	int i;
	i=strlen(path);
check:
	if(is_path_directory(path))
		return TRUE;
	while(i>2){
		i--;
		if(path[i]=='\\'){
			path[i+1]=0;
			goto check;
		}
	}
	return FALSE;
}
int CALLBACK BrowseCallbackProc(HWND hwnd,UINT msg,LPARAM lparam,LPARAM lpdata)
{
	switch(msg){
	case BFFM_INITIALIZED:
		SendMessage(hwnd,BFFM_SETSELECTION,TRUE,lpdata);
		break;
	}
	return 0;
}
int handle_browse_dialog(HWND hwnd,char *path)
{
	ITEMIDLIST *pidl;
	BROWSEINFO bi;
	IMalloc	*palloc;
	int result=FALSE;
	memset(&bi,0,sizeof(bi));
	bi.hwndOwner=hwnd;
	bi.ulFlags=BIF_EDITBOX|0x40; //BIF_NEWDIALOGSTYLE
	if(path[0]!=0){
		if(find_valid_path(path)){
			bi.lpfn=BrowseCallbackProc;
			bi.lParam=(long)path;
		}
	}
	pidl=SHBrowseForFolder(&bi);
	if(pidl!=0){
		result=SHGetPathFromIDList(pidl,path);
		if(SHGetMalloc(&palloc)==NOERROR)
		{
			palloc->lpVtbl->Free(palloc,pidl);
			palloc->lpVtbl->Release(palloc);
		}

	}
	return result;
}
int add_path(HWND hwnd,char *path)
{
	int i;
	if(strlen(path)>0){
		if(get_max_dirs()>SendDlgItemMessage(hwnd,IDC_DEFAULT_DIR,CB_GETCOUNT,0,0)){
			if(CB_ERR==SendDlgItemMessage(hwnd,IDC_DEFAULT_DIR,CB_FINDSTRINGEXACT,0,path)){
				SendDlgItemMessage(hwnd,IDC_DEFAULT_DIR,CB_ADDSTRING,0,path);
				i=SendDlgItemMessage(hwnd,IDC_DEFAULT_DIR,CB_FINDSTRINGEXACT,0,path);
				SendDlgItemMessage(hwnd,IDC_DEFAULT_DIR,CB_SETCURSEL,i,0);
				return TRUE;
			}
		}
	}
	return FALSE;
}
int save_item(HWND hwnd,char *key,int control)
{
	char str[256];
	str[0]=0;
	GetWindowText(GetDlgItem(hwnd,control),str,sizeof(str));
	return write_ini_str(key,str);
}
int save_settings(HWND hwnd)
{
	char key[100],path[MAX_PATH];
	int i;
	save_item(hwnd,"news_server",IDC_NEWS_SERVER);
	save_item(hwnd,"news_port",IDC_PORT);
	save_item(hwnd,"user_name",IDC_USERNAME);
	save_item(hwnd,"password",IDC_PASSWORD);
	save_item(hwnd,"threads",IDC_THREADS);
	for(i=0;i<get_max_dirs();i++){
		_snprintf(key,sizeof(key),"default_dir%i",i+1);
		path[0]=0;
		if(CB_ERR!=SendDlgItemMessage(hwnd,IDC_DEFAULT_DIR,CB_GETLBTEXT,i,path))
			write_ini_str(key,path);
		else
			write_ini_str(key,path);
	}
	return 0;
}
int check_settings(HWND hwnd)
{
	int i,changed=FALSE;
	char msg[256];
	struct SETTINGS{char *key;int idc;};
	struct SETTINGS settings[]={
		{"news_server",IDC_NEWS_SERVER},
		{"news_port",IDC_PORT},
		{"user_name",IDC_USERNAME},
		{"password",IDC_PASSWORD},
		{"threads",IDC_THREADS}};
	memset(msg,0,sizeof(msg));
	sprintf(msg,"settings were changed but not saved:\r\n");
	for(i=0;i<sizeof(settings)/sizeof(struct SETTINGS);i++){
		char str1[256],str2[256];
		str1[0]=0;str2[0]=0;
		GetWindowText(GetDlgItem(hwnd,settings[i].idc),str1,sizeof(str1));
		get_ini_str(settings[i].key,str2,sizeof(str2));
		if(strcmp(str1,str2)!=0){
			strncat(msg,settings[i].key,sizeof(msg)-1);
			strncat(msg,"\r\n",sizeof(msg)-1);
			changed=TRUE;
		}
	}
	for(i=0;i<get_max_dirs();i++){
		char path1[MAX_PATH],path2[MAX_PATH],key[100];
		_snprintf(key,sizeof(key),"default_dir%i",i+1);
		path1[0]=0;path2[0]=0;
		SendDlgItemMessage(hwnd,IDC_DEFAULT_DIR,CB_GETLBTEXT,i,path1);
		get_ini_str(key,path2,sizeof(path2));
		if(strcmp(path1,path2)!=0){
			strncat(msg,key,sizeof(msg)-1);
			strncat(msg,"\r\n",sizeof(msg)-1);
			changed=TRUE;
		}
	}
	if(changed){
		if(MessageBox(hwnd,msg,"Save Settings?",MB_OKCANCEL)==IDOK){
			//save_settings(hwnd);
		}
	}
}
int load_item(HWND hwnd,char *key,int control)
{
	char str[256];
	str[0]=0;
	get_ini_str(key,str,sizeof(str));
	SetWindowText(GetDlgItem(hwnd,control),str);
	return 0;
}
int load_settings(HWND hwnd)
{
	char key[100],path[MAX_PATH];
	int i;
	load_item(hwnd,"news_server",IDC_NEWS_SERVER);
	load_item(hwnd,"news_port",IDC_PORT);
	load_item(hwnd,"news_server",IDC_NEWS_SERVER);
	load_item(hwnd,"user_name",IDC_USERNAME);
	load_item(hwnd,"password",IDC_PASSWORD);
	load_item(hwnd,"threads",IDC_THREADS);
	for(i=0;i<get_max_dirs();i++){
		_snprintf(key,sizeof(key),"default_dir%i",i+1);
		path[0]=0;
		if(get_ini_str(key,path,sizeof(path)))
			SendDlgItemMessage(hwnd,IDC_DEFAULT_DIR,CB_ADDSTRING,0,path);
	}
	SendDlgItemMessage(hwnd,IDC_DEFAULT_DIR,CB_SETCURSEL,0,0);
	return 0;

}
int load_window_ini(HWND hwnd)
{
	char str[20];
	RECT rect;
	int width=0,height=0,x=0,y=0;
	get_ini_value("window_width",&width);
	get_ini_value("window_height",&height);
	get_ini_value("window_xpos",&x);
	get_ini_value("window_ypos",&y);
	str[0]=0;
	if(get_ini_str("window_maximized",str,sizeof(str))){
		if(strcmp(str,"true")==0){
			ShowWindow(hwnd,SW_SHOWMAXIMIZED);
			return TRUE;
		}
	}
	if(GetWindowRect(GetDesktopWindow(),&rect)!=0){
		int flags=SWP_SHOWWINDOW;
		if(width<50 || height<50)
			flags|=SWP_NOSIZE;
		if(x<-32 || y<=-32)
			flags|=SWP_NOMOVE;
		if(x<((rect.right-rect.left)-50))
			if(y<((rect.bottom-rect.top)-50))
				if(SetWindowPos(hwnd,HWND_TOP,x,y,width,height,flags)!=0)
					return TRUE;
	}
	return FALSE;
}
int save_window_ini(HWND hwnd)
{
	char str[20];
	RECT rect;
	WINDOWPLACEMENT wp;
	wp.length=sizeof(wp);
	if(GetWindowPlacement(hwnd,&wp)!=0){
		if(wp.flags&WPF_RESTORETOMAXIMIZED)
			write_ini_str("window_maximized","true");
		else
			write_ini_str("window_maximized","false");
	}
	if(GetWindowRect(hwnd,&rect)!=0){
		str[0]=0;
		_snprintf(str,sizeof(str),"%i",rect.right-rect.left);
		write_ini_str("window_width",str);
		_snprintf(str,sizeof(str),"%i",rect.bottom-rect.top);
		write_ini_str("window_height",str);
		_snprintf(str,sizeof(str),"%i",rect.left);
		write_ini_str("window_xpos",str);
		_snprintf(str,sizeof(str),"%i",rect.top);
		write_ini_str("window_ypos",str);
		return TRUE;
	}
	return FALSE;
}
extern short settings_anchors[];
BOOL CALLBACK settings_dlg(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	int i,j;
	char path[MAX_PATH];
	static RECT drag;
	static DWORD tick,mousetick=0;
	static HWND grippy=0;
#ifdef _DEBUG
	if(FALSE)
	if(msg!=WM_MOUSEFIRST&&msg!=WM_NCHITTEST&&msg!=WM_SETCURSOR&&msg!=WM_ENTERIDLE&&msg!=WM_NOTIFY)
	{
		if((GetTickCount()-tick)>500)
			printf("--\n");
		printf(">");
		print_msg(msg,lparam,wparam);
		tick=GetTickCount();
	}
#endif
	switch(msg)
	{
	case WM_INITDIALOG:
		grippy=create_grippy(hwnd);
		SendDlgItemMessage(hwnd,IDC_DEFAULT_DIR,CB_LIMITTEXT,MAX_PATH-1,0);
		SendDlgItemMessage(hwnd,IDC_NEWS_SERVER,EM_LIMITTEXT,256,0);
		SendDlgItemMessage(hwnd,IDC_PORT,EM_LIMITTEXT,200,0);
		SendDlgItemMessage(hwnd,IDC_USERNAME,EM_LIMITTEXT,200,0);
		SendDlgItemMessage(hwnd,IDC_PASSWORD,EM_LIMITTEXT,200,0);
		SendDlgItemMessage(hwnd,IDC_THREADS,EM_LIMITTEXT,2,0);
		load_settings(hwnd);
		load_icon(hwnd);
		break;
	case WM_SIZE:
		grippy_move(hwnd,grippy);
		reposition_controls(hwnd,settings_anchors);
		break;
	case WM_SIZING:
		break;
	case WM_USER:
		//if(HIWORD(wparam)==CBN_SELCHANGE)
		{
			char str[10];
			i=SendDlgItemMessage(hwnd,IDC_DEFAULT_DIR,CB_GETCURSEL,0,0);
			if(i!=CB_ERR)
				_snprintf(str,sizeof(str),"%i",i+1);
			else
				str[0]=0;
			SetWindowText(GetDlgItem(hwnd,IDC_LIST_NUM),str);
		}
		break;
	case WM_MOUSEACTIVATE:
		mousetick=GetTickCount();
		break;
	case WM_COMMAND:
		switch(LOWORD(wparam))
		{
		case IDC_NEWS_SERVER:
			break;
		case IDC_PORT:
			break;
		case IDC_USERNAME:
			break;
		case IDC_PASSWORD:
			break;
		case IDC_DEFAULT_DIR:
			SendMessage(hwnd,WM_USER,0,0);
			break;
		case IDC_SELECT_DIR:
			if(get_max_dirs()<=SendDlgItemMessage(hwnd,IDC_DEFAULT_DIR,CB_GETCOUNT,0,0)){
				MessageBox(hwnd,"Allready have max directories","Error",MB_OK);
				break;
			}
			path[0]=0;
			if(handle_browse_dialog(hwnd,path))
				if(add_path(hwnd,path))
					SendMessage(hwnd,WM_USER,0,0);
			break;
		case IDC_DL_DIR_UP:
			i=SendDlgItemMessage(hwnd,IDC_DEFAULT_DIR,CB_GETCURSEL,0,0);
			if((i!=CB_ERR) && (i>0)){
				GetWindowText(GetDlgItem(hwnd,IDC_DEFAULT_DIR),path,sizeof(path));
				SendDlgItemMessage(hwnd,IDC_DEFAULT_DIR,CB_DELETESTRING,i,0);
				SendDlgItemMessage(hwnd,IDC_DEFAULT_DIR,CB_INSERTSTRING,i-1,path);
				SendDlgItemMessage(hwnd,IDC_DEFAULT_DIR,CB_SETCURSEL,i-1,0);
				SendMessage(hwnd,WM_USER,0,0);
			}
			break;
		case IDC_DL_DIR_DOWN:
			i=SendDlgItemMessage(hwnd,IDC_DEFAULT_DIR,CB_GETCURSEL,0,0);
			j=SendDlgItemMessage(hwnd,IDC_DEFAULT_DIR,CB_GETCOUNT,0,0);
			if((i!=CB_ERR) && (i<(j-1))){
				GetWindowText(GetDlgItem(hwnd,IDC_DEFAULT_DIR),path,sizeof(path));
				SendDlgItemMessage(hwnd,IDC_DEFAULT_DIR,CB_DELETESTRING,i,0);
				SendDlgItemMessage(hwnd,IDC_DEFAULT_DIR,CB_INSERTSTRING,i+1,path);
				SendDlgItemMessage(hwnd,IDC_DEFAULT_DIR,CB_SETCURSEL,i+1,0);
				SendMessage(hwnd,WM_USER,0,0);
			}
			break;
		case IDC_DL_DIR_ADD:
			memset(path,0,sizeof(path));
			GetWindowText(GetDlgItem(hwnd,IDC_DEFAULT_DIR),path,sizeof(path));
			add_path(hwnd,path);
			SendMessage(hwnd,WM_USER,0,0);
			break;
		case IDC_DL_DIR_DELETE:
			i=SendDlgItemMessage(hwnd,IDC_DEFAULT_DIR,CB_GETCURSEL,0,0);
			if(i!=CB_ERR){
				SendDlgItemMessage(hwnd,IDC_DEFAULT_DIR,CB_DELETESTRING,i,0);
				SendDlgItemMessage(hwnd,IDC_DEFAULT_DIR,CB_SETCURSEL,0,0);
				SendMessage(hwnd,WM_USER,0,0);
			}
			break;
		case IDC_OPEN_INI:
			open_ini(hwnd);
			break;
		case IDC_THREADS:
			if(HIWORD(wparam)==EN_CHANGE)
			{
				char str[8];str[sizeof(str)-1]=0;
				GetWindowText(GetDlgItem(hwnd,IDC_THREADS),str,sizeof(str)-1);
				i=atoi(str);
				if(i>MAX_THREADS || (i==0 && str[0]=='0')){
					if(i>MAX_THREADS)i=MAX_THREADS;
					if(i<=0)i=1;
					_snprintf(str,sizeof(str)-1,"%i",i);
					SetWindowText(GetDlgItem(hwnd,IDC_THREADS),str);
				}
			}
			break;
		case WM_DESTROY:
			if((GetTickCount()-mousetick)>100)
				check_settings(hwnd);
			EndDialog(hwnd,0);
			break;
		case IDOK:
			if(GetFocus()==GetDlgItem(hwnd,IDOK)){
				save_settings(hwnd);
				save_list_ini();
				save_window_ini(hwindow);
				create_popup_menus();
				EndDialog(hwnd,0);
			}
			else
				PostMessage(hwnd,WM_NEXTDLGCTL,0,0);
			break;
		}
		break;
		
	case WM_CLOSE:
	case WM_QUIT:
		EndDialog(hwnd,0);
		break;
	}
	return 0;
}
int update_title(char *fmt,...)
{
	va_list args;
	char str[256];
	memset(str,0,sizeof(str));
	va_start(args,fmt);
	_vsnprintf(str,sizeof(str)-1,fmt,args);
	return SetWindowText(hwindow,str);
}
int update_status(char *fmt,...)
{
	va_list args;
	char str[256];
	memset(str,0,sizeof(str));
	va_start(args,fmt);
	_vsnprintf(str,sizeof(str)-1,fmt,args);
	return SetDlgItemText(hwindow,IDC_STATUS,str);
}
static int halt_threads=TRUE;
int stop_everything()
{
	return halt_threads;
}
int unset_go_button()
{
	if(IsDlgButtonChecked(hwindow,IDC_GONOW)){
		CheckDlgButton(hwindow,IDC_GONOW,BST_UNCHECKED);
		Beep(300,80);
	}
	return TRUE;
}
int load_icon(HWND hwnd)
{
	HICON hIcon = LoadIcon(ghinstance,MAKEINTRESOURCE(IDI_ICON1));
    if(hIcon){
		SendMessage(hwnd,WM_SETICON,ICON_SMALL,(LPARAM)hIcon);
		SendMessage(hwnd,WM_SETICON,ICON_BIG,(LPARAM)hIcon);
		return TRUE;
	}
	return FALSE;
}
int is_ok_quit(HWND hwnd,char *reason)
{
	if(IsDlgButtonChecked(hwnd,IDC_GONOW))
	{
		if(MessageBox(hwnd,reason,"uNZB Quit?",MB_OKCANCEL|MB_SYSTEMMODAL)==IDOK)
			return TRUE;
		else
			return FALSE;
	}
	return TRUE;
}
extern short main_dlg_anchors[];
LRESULT CALLBACK MainDlg(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	static HWND grippy=0;
	static HWND	hlistview=0;
	static DWORD tick,help=FALSE;

#ifdef _DEBUG
	if(FALSE)
	if(msg!=WM_MOUSEFIRST&&msg!=WM_NCHITTEST&&msg!=WM_SETCURSOR&&msg!=WM_ENTERIDLE/*&&msg!=WM_NOTIFY*/)
	//if(msg!=WM_NCHITTEST&&msg!=WM_SETCURSOR&&msg!=WM_ENTERIDLE)
	{
		if((GetTickCount()-tick)>500)
			printf("--\n");
		printf("*");
		print_msg(msg,lparam,wparam);
		tick=GetTickCount();
	}
#endif	
	switch(msg)
	{
	case WM_INITDIALOG:
		hwindow=hwnd; //go ahead init it here
		hlistview=CreateListView(hwnd);
		grippy=create_grippy(hwnd);
		BringWindowToTop(hwnd);
		load_listview_file();
		//create_network_thread(hwnd,&event);
		create_threads(hwnd);
		load_icon(hwnd);
		break;
	case WM_DROPFILES:
		handle_file_drop((HANDLE)wparam);
		break;
	case WM_HELP:
		if(!help){
			help=TRUE;
			MessageBox(hwnd,"F1=Help\r\n1-0=directory hotlist 1-10\r\n"
				"tilde or D=open directory browse dialog\r\n"
				"R=reset incompletes\r\n"
				"CTRL+R=reset status\r\n"
				"BACKSPACE=set directory up one level\r\n"
				"INSERT=Load NZB\r\n"
				"CTRL+S=Settings\r\n"
				"CTRL+C=Halt download\r\n"
				"F5=GO\r\n",
				"Help",MB_OK);
			help=FALSE;
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wparam))
		{
		case IDC_GONOW:
			if(IsDlgButtonChecked(hwnd,IDC_GONOW)){
				if(is_all_dl_paths_set()){
					halt_threads=FALSE;
					trigger_threads(TRUE);
				}else{
					update_status("not all download paths are set");
					CheckDlgButton(hwnd,IDC_GONOW,BST_UNCHECKED);
				}

			}
			else
				halt_threads=TRUE;
			break;
		case IDC_TEST2:
			break;
		case IDC_TEST:
			break;
		case IDC_LOADNZB:
			handle_file_dialog(hwnd);
			break;
		case IDC_SETTINGS:
			DialogBox(ghinstance,IDD_SETTINGS,hwnd,settings_dlg);
			break;
		case IDOK:
			break;
		case WM_DESTROY:
			save_list_ini();
			save_window_ini(hwnd);
			save_listview();
		#ifndef _DEBUG
			if(!is_ok_quit(hwnd,"Sure you want to quit?"))
				break;
		#endif
			PostQuitMessage(0);
			break;
		}
		break;
	case WM_MOUSEWHEEL:
		if(GetFocus()!=hlistview)
			SendMessage(hlistview,msg,wparam,lparam);
		break;
	case WM_KEYFIRST:
		switch(LOWORD(wparam)){
		case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
			if(!any_threads_busy()){
				int i=LOWORD(wparam)-'0';
				char key[40],path[MAX_PATH]={0};
				if(i==0)i==10;
				memset(key,0,sizeof(key));
				_snprintf(key,sizeof(key)-1,"default_dir%i",i);
				get_ini_str(key,path,sizeof(path));
				if(path[0]!=0)
					update_selected_paths(path);
			}
			break;
		case 'a':
		case 'A':
			if(GetKeyState(VK_CONTROL)&0x8000)
				select_all();
			break;
		case 'r':
		case 'R':
			if(!any_threads_busy()){
				if(GetKeyState(VK_CONTROL)&0x8000)
					reset_selected_items();
				else
					reset_incomplete_items();
			}
			break;
		case 's':
		case 'S':
			if(GetKeyState(VK_CONTROL)&0x8000)
				SendMessage(hwnd,WM_COMMAND,IDC_SETTINGS,0);
			break;
		case 'c':
		case 'C':
			if(GetKeyState(VK_CONTROL)&0x8000)
				if(any_threads_busy())
					halt_threads=TRUE;
			break;
		case 'd':
		case 'D':
		case 0xC0: //"`"
			if(!any_threads_busy())
				set_dl_folder(hwnd);
			break;
		case VK_BACK:
			if(!any_threads_busy())
				go_up_one_level();
			break;
		case VK_F5:
			if(!any_threads_busy()){
				CheckDlgButton(hwnd,IDC_GONOW,BST_CHECKED);
				SendMessage(hwnd,WM_COMMAND,IDC_GONOW,0);
			}
			break;
		case VK_INSERT:
			SendMessage(hwnd,WM_COMMAND,IDC_LOADNZB,0);
			break;
		case VK_DELETE:
			if(items_selected()==0)
				break;
			if(MessageBox(hwnd,"Sure you want to delete selected items","DELETE",MB_OKCANCEL)!=IDOK)
				break;
			delete_items();
			update_byte_counters();
			break;
		case VK_PRIOR:
		case VK_NEXT:
		case VK_UP:
		case VK_DOWN:
		case VK_LEFT:
		case VK_RIGHT:
		case VK_HOME:
		case VK_END:
			if(GetFocus()!=hlistview){
				SetFocus(hlistview);
				SendMessage(hlistview,msg,wparam,lparam);
			}
			break;
		}
		break;
	case WM_QUERYENDSESSION:
		if(!is_ok_quit(hwnd,"End session detected\r\nDownload in progress, ok to quit?"))
			return 1;
		else
			return 0;
		break;
	case WM_ENDSESSION:
		if(wparam){
			int count=0;
			while(any_threads_busy()){
				halt_threads=TRUE;
				Sleep(1000);
				count++;
				if(count>10)
					break;
			}
			save_list_ini();
			save_window_ini(hwnd);
			save_listview();
		}
		return 0;
		break;
	case WM_CLOSE:
	case WM_QUIT:
		break;
	case WM_WINDOWPOSCHANGED:
		//printf("%i %i\n",((LPWINDOWPOS)lparam)->x,((LPWINDOWPOS)lparam)->y);
		break;
	case WM_SIZE:
		grippy_move(hwnd,grippy);
		reposition_controls(hwnd,main_dlg_anchors);
		break;
	}
	return 0;
}

int check_debug()
{
#ifdef _DEBUG
		set_debug_level(3);
		return TRUE;
#endif
	if(does_file_exist("debug")){
		set_debug_level(3);
		return TRUE;
	}
	else{
		int val=0;
		get_ini_value("debug",&val);
		if(val>0){
			set_debug_level(val);
			return TRUE;
		}
	}
	return FALSE;
}
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
extern HWND hlistview;
	MSG msg;
	ghinstance=hInstance;
	init_ini_file();
	if(check_debug()){
		open_console();
		move_console();
	}
	hwindow=CreateDialog(ghinstance,MAKEINTRESOURCE(IDD_DIALOG1),NULL,(DLGPROC)MainDlg);
	if(!hwindow){
		MessageBox(NULL,"Could not create main dialog","ERROR",MB_ICONERROR | MB_OK);
		return 0;
	}
	ShowWindow(hwindow,nCmdShow);
	if(!load_window_ini(hwindow))
		ShowWindow(hwindow,nCmdShow);
	UpdateWindow(hwindow);

	while(GetMessage(&msg,NULL,0,0))
	{
		if(!IsDialogMessage(hwindow,&msg)){		// Translate messages for the dialog
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else{
		//	debug_printf("msg.message=%08X msg.lparam=%08X msg.wparam=%08X\n",msg.message,msg.lParam,msg.wParam);
		//	DispatchMessage(&msg);
			//if(msg.message == WM_KEYDOWN && msg.wParam!=VK_ESCAPE){
			if((msg.message == WM_KEYDOWN && msg.wParam!=VK_ESCAPE && msg.wParam!=VK_SHIFT && msg.wParam!=VK_CONTROL)
				|| (msg.message==WM_KEYUP && msg.wParam!=VK_ESCAPE && msg.wParam!=VK_SHIFT && msg.wParam!=VK_CONTROL)){
				static DWORD time=0;
				//if((GetTickCount()-time)>100){
				{
					//if(screen_updated)
					{	
						screen_updated=FALSE;
						SendMessage(hwindow,msg.message,msg.wParam,msg.lParam);
						time=GetTickCount();
					}
				}
			}
		}
	}
	return msg.wParam;
}



