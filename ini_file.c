#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <shlwapi.h>
#include <Shlobj.h>


#define INI_FNAME "UNZB.ini"

char ini_file[MAX_PATH]={0};
int get_listview_file(char *path,int len)
{
	char str[MAX_PATH];
	strncpy(str,ini_file,len);
	if(find_valid_path(str)){
		_snprintf(path,len,"%s\\%s",str,"list_save.nzb");
		return TRUE;
	}
	return FALSE;
}
int load_listview_file()
{
	char fname[MAX_PATH];
	if(get_listview_file(fname,sizeof(fname))){
		load_nzb_file(fname);
		return TRUE;
	}
	return FALSE;
}
int get_ini_value(char *key,int *val)
{
	char str[255];
	int result=FALSE;
	str[0]=0;
	result=GetPrivateProfileString("UNZB",key,"",str,sizeof(str),ini_file);
	if(str[0]!=0)
		*val=atoi(str);
	return result>0;
}
int get_ini_str(char *key,char *str,int size)
{
	int result=FALSE;
	char tmpstr[1024];
	tmpstr[0]=0;
	result=GetPrivateProfileString("UNZB",key,"",tmpstr,sizeof(tmpstr),ini_file);
	if(result>0)
		strncpy(str,tmpstr,size);
	return result>0;
}
int write_ini_str(char *key,char *str)
{
	return WritePrivateProfileString("UNZB",key,str,ini_file);
}
int add_trail_slash(char *path)
{
	int i;
	i=strlen(path);
	if(i>0 && path[i-1]!='\\')
		strcat(path,"\\");
	return TRUE;
}
int create_directory(char *path)
{
	char tmp[MAX_PATH];
	int i,len,result=TRUE;
	len=strlen(path);
	for(i=3;i<len;i++){
		if(path[i]=='\\'){
			memset(tmp,0,sizeof(tmp));
			strncpy(tmp,path,i);
			if(!is_path_directory(tmp))
				if(CreateDirectory(tmp,NULL)==0)
					result=FALSE;
		}
	}
	if(!is_path_directory(path))
		if(CreateDirectory(path,NULL)==0)
			result=FALSE;
	return result;
}

int does_file_exist(char *fname)
{
	FILE *f;
	f=fopen(fname,"rb");
	if(f!=0){
		fclose(f);
		return TRUE;
	}
	return FALSE;
}
int get_appdata_folder(char *path)
{
	int found=FALSE;
	ITEMIDLIST *pidl;
	IMalloc	*palloc;
	extern HWND hwindow;
	if(SHGetSpecialFolderLocation(hwindow,CSIDL_APPDATA,&pidl)==NOERROR){
		if(SHGetPathFromIDList(pidl,path)){
			found=TRUE;
		}
		if(SHGetMalloc(&palloc)==NOERROR){
			palloc->lpVtbl->Free(palloc,pidl);
			palloc->lpVtbl->Release(palloc);
		}
	}
	return found;
}
int create_portable_file()
{
	FILE *f;
	f=fopen("UNZB_portable","wb");
	if(f!=0){
		fclose(f);
		return TRUE;
	}
	return FALSE;
}
int init_ini_file()
{
	char path[MAX_PATH],str[MAX_PATH];
	FILE *f;
	memset(ini_file,0,sizeof(ini_file));
	memset(path,0,sizeof(path));
	memset(str,0,sizeof(str));
	GetCurrentDirectory(sizeof(path),path);
	_snprintf(str,sizeof(str)-1,"%s\\UNZB_portable",path);
	if(does_file_exist(str)){
		if(!does_file_exist(INI_FNAME))
			goto install;
	}
	else{
		if(get_appdata_folder(path)){
			add_trail_slash(path);
			strcat(path,"UNZB\\");
			_snprintf(str,sizeof(str)-1,"%s%s",path,INI_FNAME);
			if((!is_path_directory(path)) || (!does_file_exist(str))){
				char str[MAX_PATH*3],cdir[MAX_PATH];
				memset(str,0,sizeof(str));memset(cdir,0,sizeof(cdir));
				GetCurrentDirectory(sizeof(cdir),cdir);
				_snprintf(str,sizeof(str)-1,"YES=Create directory %s to put INI file in\r\n\r\n"
					"NO=Make installation portable by putting INI in current directory %s\r\n\r\n"
					"CANCEL=Abort installation",path,cdir);
				switch(MessageBox(NULL,str,"Install",MB_YESNOCANCEL|MB_SYSTEMMODAL)){
				default:
				case IDCANCEL:
					exit(-1);
					break;
				case IDYES:
					CreateDirectory(path,NULL);
					break;
				case IDNO:
					GetCurrentDirectory(sizeof(path),path);
					create_portable_file();
					break;
				}
			}
		}else{
			char str[MAX_PATH*3];
install:
			GetCurrentDirectory(sizeof(path),path);
			memset(str,0,sizeof(str));
			_snprintf(str,sizeof(str)-1,
				"OK=Install INI in current directory %s\r\n\r\n"
				"CANCEL=Abort installation",path);
			switch(MessageBox(NULL,str,"Install",MB_OKCANCEL|MB_SYSTEMMODAL)){
			default:
			case IDCANCEL:
				exit(-1);
				break;
			case IDOK:
				create_portable_file();
				break;
			}
		}

	}
	add_trail_slash(path);
	_snprintf(ini_file,sizeof(ini_file)-1,"%s%s",path,INI_FNAME);
	f=fopen(ini_file,"rb");
	if(f==0){
		f=fopen(ini_file,"wb");
	}
	if(f!=0){
		char str[255];
		fclose(f);
		str[0]=0;
		get_ini_str("column_width_filename",str,sizeof(str));
		if(str[0]==0)
			write_new_list_ini();
	}
	restore_last_path();
	return 0;
}

int open_ini(HWND hwnd)
{
	WIN32_FIND_DATA fd;
	HANDLE h;
	char str[MAX_PATH+80];
	if(h=FindFirstFile(ini_file,&fd)!=INVALID_HANDLE_VALUE){
		FindClose(h);
		ShellExecute(0,"open","notepad.exe",ini_file,NULL,SW_SHOWNORMAL);
	}
	else if(hwnd!=0){
		memset(str,0,sizeof(str));
		_snprintf(str,sizeof(str)-1,"cant locate ini file:\r\n%s",ini_file);
		MessageBox(hwnd,str,"Error",MB_OK);
	}
	return TRUE;
}
