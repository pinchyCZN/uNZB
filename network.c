#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#include "polarssl/config.h"

#include "polarssl/net.h"
#include "polarssl/ssl.h"
#include "polarssl/entropy.h"
#include "polarssl/ctr_drbg.h"
#include "polarssl/error.h"
/*
AUTHINFO USER 123

AUTHINFO PASS 456

group alt.binaries.boneless
article <tvudnQVb1NlzCxzSnZ2dnUVZ5radnZ2d@giganews.com>

ARTICLE ident - Displays an entire article. Ident can either be a message id or an article number inside the current group. 
HEAD ident - Displays the headers of an article. 
BODY ident - Displays the body of an article. 
GROUP groupid - Sets the current group and displays number of messages, low and high article numbers, and postability of that group 
LIST - Lists all newsgroups, their low and high article numbers, and their postability
1xx - Informative message
2xx - Command ok
3xx - Command ok so far, send the rest of it.
4xx - Command was correct, but couldn't be performed for
    some reason.
5xx - Command unimplemented, or incorrect, or a serious
    program error occurred.

x0x - Connection, setup, and miscellaneous messages
x1x - Newsgroup selection
x2x - Article selection
x3x - Distribution functions
x4x - Posting
x8x - Nonstandard (private implementation) extensions
x9x - Debugging output
*/
#define MAX_THREADS 10
#define STATUS_EVENT MAX_THREADS
#define MAX_RETRYS 5
enum{SEGMENT_BAD=-1,SEGMENT_NOT_AVAIL=-2,SEGMENT_TIMEOUT=-3};
HANDLE events[MAX_THREADS+1];
int threads_busy=0,thread_status=0;
enum{THREAD_DONE,THREAD_ABORTED,THREAD_LOST,THREAD_NC,THREAD_CREDS};
CRITICAL_SECTION cs;

unsigned __int64 list_total=0,download_total=0;
DWORD delta_total=0;

int update_byte_counters()
{
	list_total=0;download_total=0;
	get_total_bytes(&list_total,&download_total);
	if(!any_threads_busy())
		update_status("list total=%.3f MB",((double)list_total)/1024/1024);
	return TRUE;
}
int socket_speed(unsigned int len)
{
	download_total+=len;
	return TRUE;
}
int update_speed_status(int init)
{
	static DWORD tick1=0,tick2=0,avg_index=0;
	static double avgs[10]={0,0,0,0,0,0,0,0,0,0};
	static __int64 total=0,lasttotal=0;
	DWORD delta;
	if(init){
		total=lasttotal=0;
		memset(avgs,0,sizeof(avgs));
		return 0;
	}
	total=download_total;
	tick1=GetTickCount();
	delta=tick1-tick2;
	if(delta>=1000){
		double percent,remain,eta,speed=1;
		speed=((double)(total-lasttotal))/(double)(delta);
		speed*=1000; //B/s
		remain=list_total-download_total;
		percent=(double)download_total*100/(double)list_total;
		if(speed!=0 && list_total!=0){
			double avs=0;int i;
			avgs[avg_index++]=speed;
			avg_index%=(sizeof(avgs)/sizeof(double));
			for(i=0;i<(sizeof(avgs)/sizeof(double));i++)
				avs+=avgs[i];
			avs/=(sizeof(avgs)/sizeof(double));
			eta=remain/avs/60; //minutes
			if(eta>20000){
				update_status("%4.1f KB/s  eta:long long time (%3.1f%%) \t\ttotal=%.3f MB",avs/1024,percent,((double)list_total)/1024/1024);
				update_title("uNZB (%3.1f%%)",percent);
			}
			else{
				update_status("%4.1f KB/s  %4.2f min (%3.1f%%) \t\ttotal=%.3f MB",avs/1024,eta,percent,((double)list_total)/1024/1024);
				update_title("uNZB (%3.1f%% %4.2f min)",percent,eta);
			}
		}
		else
			update_status("0 KB/s forever (%4.2f) \t\ttotal=%.3f MB",percent,((double)list_total)/1024/1024);
		lasttotal=total;
		tick2=tick1;
	}
	return TRUE;
}
int DEBUG_LEVEL=0;
int set_debug_level(int i)
{ DEBUG_LEVEL=i;return 0; }
void my_debug(void *ctx,int level,const char *str)
{
	if(level<1){
		fprintf((FILE*)ctx,"%s",str);
		fflush((FILE*)ctx);
	}
}
int debug_pf(int level,char *fmt,...)
{
	va_list args;
	if(level<DEBUG_LEVEL){
		va_start(args,fmt);
		vprintf(fmt,args);
	}
	return 0;
}
int is_socket_alive(int *socket)
{
	int result;
	fd_set r;
	TIMEVAL t;
	if(socket==0)return FALSE;
	FD_ZERO(&r);
	FD_SET(*socket,&r);
	t.tv_sec=0;
	t.tv_usec=0;
	result=select(0,&r,0,0,&t);
	if(result>=1){
		int data=0;
		ioctlsocket(*socket,FIONREAD,&data);
		if(data==0)
			return FALSE;
		else
			return TRUE;
	}
	else if(result!=SOCKET_ERROR)
		return TRUE;
	else
		return FALSE;
}
int can_read(int *socket,int seconds)
{
	fd_set r;
	TIMEVAL t;
	if(socket==0)return FALSE;
	FD_ZERO(&r);
	FD_SET(*socket,&r);
	t.tv_sec=seconds;
	t.tv_usec=0;
	if(select(0,&r,0,0,&t)>=1){
		int data=0;
		ioctlsocket(*socket,FIONREAD,&data);
		if(data==0)
			return FALSE;
		else
			return TRUE;
	}
	return FALSE;
}
int can_write(int *socket,int seconds)
{
	fd_set w;
	TIMEVAL t;
	if(socket==0)return FALSE;
	FD_ZERO(&w);
	FD_SET(*socket,&w);
	t.tv_sec=seconds;
	t.tv_usec=0;
	if(select(0,0,&w,0,&t)>=1){
		return TRUE;
	}
	return FALSE;
}
int send_and_check(ssl_context *ssl,char *str,int max,char *check)
{
	int index=0,timeout=0;
	if(can_write(ssl->p_send,0)){
		debug_pf(2,"%08X:%s",ssl->p_send,str);
		ssl_write(ssl,str,strlen(str));
	}
	else
		return FALSE;
	memset(str,0,max);
	while(strchr(str,'\n')==0){
		if(can_read(ssl->p_recv,1)){
			int read=0;
			read=ssl_read(ssl,str+index,max-index);
			if(read>0)
				index+=read;
			else
				return FALSE;
			if(index>=max-1)
				break;
		}
		else
			timeout++;
		if(timeout>10)
			return FALSE;
	}
	debug_pf(2,"%08X:%s",ssl->p_send,str);
	if(check==NULL)
		return TRUE;
	else{
		int i,start=FALSE,found=TRUE,index=0;
		for(i=0;i<max;i++){
			if(check[index]==0)
				break;
			if(str[i]>='0' && str[i]<='9'){
				start=TRUE;
				if(check[index]=='x')
					index++;
				else if(check[index++]!=str[i]){
					found=FALSE;
					break;
				}
			}
			else if(start){
				found=FALSE;
				break;
			}
		}
		if(!start)found=FALSE;
		return found;
	}
	return FALSE;
}
int check_basic_command(ssl_context *ssl)
{
	char str[100];
	_snprintf(str,sizeof(str),"GROUP alt.binaries.test\n");
	if(send_and_check(ssl,str,sizeof(str),"2xx")){
		return TRUE;
	}
	return FALSE;
}
int connect_news_server(char *name,int port,ssl_context *ssl)
{
	int ret;
    unsigned char buf[1024];
    entropy_context entropy;
    ctr_drbg_context ctr_drbg;
    ssl_session ssn;
	int socket=0;
    char *pers="news connection";

    memset(&ssn,0,sizeof(ssl_session));
    memset(ssl,0,sizeof(ssl_context));
    entropy_init(&entropy);
    if((ret=ctr_drbg_init(&ctr_drbg,entropy_func,&entropy,
                               (unsigned char *)pers,strlen(pers)))!=0){
        debug_pf(1," failed\n ! ctr_drbg_init returned %d\n",ret);
		return FALSE;
    }
	if((ret=net_connect(&socket,name,port))!=0)
	{
		debug_pf(1,"failed\n ! net_connect returned %d\n\n",ret);
		return FALSE;
	}

    if((ret=ssl_init(ssl))!=0){
		net_close(socket);
        debug_pf(1,"failed\n ! ssl_init returned %d\n\n",ret);
		return FALSE;
    }
	ssl_set_endpoint(ssl,SSL_IS_CLIENT);
	ssl_set_authmode(ssl,SSL_VERIFY_NONE);
	
	ssl_set_rng(ssl,ctr_drbg_random,&ctr_drbg);
	ssl_set_dbg(ssl,my_debug,stdout);
	ssl_set_bio(ssl,net_recv,&socket,net_send,&socket);
	
	ssl_set_ciphersuites(ssl,ssl_default_ciphersuites);
	ssl_set_session(ssl,1,600,&ssn);
	ssl->read_timeout=30;
	get_ini_value("read_timeout",&ssl->read_timeout);

	if(can_write(&socket,1)){
		ssl_write(ssl,"\n\n\n\n",4);
		memset(buf,0,sizeof(buf));
		if(can_read(&socket,5))
			ssl_read(ssl,buf,sizeof(buf)-1);
		debug_pf(1,"first login response:%s\n",buf);
	}
	else
		debug_pf(1,"couldnt write socket right off the bat!\n");
	return TRUE;
}

int disconnect(ssl_context *ssl)
{
	if(can_write(ssl->p_send,0))
		ssl_write(ssl,"QUIT\n",sizeof("QUIT\n"));
	if(can_read(ssl->p_recv,5)){
		char buf[80];
		memset(buf,0,sizeof(buf));
		ssl_read(ssl,buf,sizeof(buf)-1);
		debug_pf(1,"quit message:%s\n",buf);
	}
	ssl_close_notify(ssl);
	net_close(ssl->p_recv);
	ssl_free(ssl);
	return TRUE;
}
int clear_socket(ssl_context *ssl)
{
	int once=TRUE;
	char buf[256];
	DWORD time=GetTickCount();
	while(can_read(ssl->p_recv,0)){
		int read;
		if(once)debug_pf(1,"clearing socket\n");
		once=FALSE;
		read=ssl_read(ssl,buf,sizeof(buf));
		if(read<=0)
			break;
		if((GetTickCount()-time)>60000)
			break;
	}
	return TRUE;
}
int send_group_cmd(int xml,ssl_context *ssl)
{
	const char *group;
	char cmd[256];
	if(get_group_name(xml,&group)){
		memset(cmd,0,sizeof(cmd));
		_snprintf(cmd,sizeof(cmd)-1,"GROUP %s\n",group);
		debug_pf(2,cmd);
		if(send_and_check(ssl,cmd,sizeof(cmd),"2xx"))
			return TRUE;
	}
	return FALSE;
}
int dl_segment(ssl_context *ssl,
			   const char *segname,char *path)
{
	int total=0,result=FALSE;
	char *src;
	char *buf=0;
	int bsize=6000*256;
	int src_size=0x10000;
	FILE *f=0;
	static int fcount=0;
	char fname[MAX_PATH];
	_snprintf(fname,sizeof(fname),"%s\\test%i.bin",path,fcount++);
	f=0;
//	f=fopen(fname,"wb"); //for debugging,saves each raw segment


	buf=malloc(bsize);
	if(buf==0){
		update_status("cant allocate memory for segment");
		return FALSE;
	}
	src=malloc(src_size);
	if(src==0){
		free(buf);
		update_status("cant allocate memory for segment");
		return FALSE;
	}
	clear_socket(ssl);
	memset(src,0,src_size);
	_snprintf(src,src_size-1,"BODY <%s>\n",segname);
	if(send_and_check(ssl,src,src_size,"2xx"))
	{
		VOID *yd=ydec_new();
		int timeout=2,retry=0,maxtrys=4;
		memset(buf,0,bsize);
		ydecode(yd,src,src_size,buf,bsize); //decode any trailing data from BODY command
		while(TRUE){
			int read=0;
			if(can_read(ssl->p_recv,timeout)){
				read=ssl_read(ssl,src,src_size);
				if(read>0){
					total+=read;
					socket_speed(read);
					if(f)fwrite(src,1,read,f);
					ydecode(yd,src,read,buf,bsize);
				}
				else if(read<0)
					retry++;
			}
			else{
				timeout=5;
				retry++;
				debug_pf(1,"timeout %08X\n",ssl->p_recv);
				if(can_write(ssl->p_send,0))
					ssl_write(ssl,".\n",2);
			}
			if(retry>maxtrys){
				result=SEGMENT_TIMEOUT;
				break;
			}
			if(is_end_message(yd))
				break;
		};
		if(yec_save_file(yd,buf,path)){
			if(retry<=maxtrys)
				result=TRUE;
			if(is_yend_reached(yd))
				if(!yenc_verify_file(yd,buf,bsize,path)){
					debug_pf(1,"invalid CRC for file\n");
					result=SEGMENT_BAD;
				}
		}
		else{
			debug_pf(1,"unable to save file\n");
		}
		ydec_delete(yd);
	}
	else{
		debug_pf(1,"cant get article body\n");
		result=SEGMENT_NOT_AVAIL;
	}
	clear_socket(ssl);

	if(f)fclose(f);
	free(buf);
	free(src);
	debug_pf(2,"done with %s\n",segname);
	return result;
}
int dl_file(ssl_context *ssl,int xml)
{
	int i,segments=0,incompletes=0;
	char path[MAX_PATH];

	path[0]=0;
	get_item_dl_path(xml,&path,sizeof(path));
	if(path[0]==0){
		update_status("dl path not set");
		return FALSE;
	}
	else if(!is_path_directory(path) && !create_directory(path)){
		update_status("cant create folder %s",path);
		return FALSE;
	}
	if(is_file_done(xml))
		return TRUE;
	if(!send_group_cmd(xml,ssl))
		return FALSE;
	segments=get_segment_count(xml);
	for(i=0;i<segments;i++){
		const char *segname=0;
		int incompl=0,downloaded=0,retry=0;
		while(retry<2){
			if(get_segment_info(xml,i,&segname,&incompl,&downloaded)){
				if(downloaded==0){
					int result=0;
					if(!claim_segment(xml,i)){
						retry=100;
						continue;
					}
					if(stop_everything()){
						release_segment(xml,i);
						return FALSE;
					}
					result=dl_segment(ssl,segname,path);
					if(result==SEGMENT_BAD){
						downloaded=TRUE;
						incompl=1;
					}
					else if(result==SEGMENT_NOT_AVAIL){
						if(incompl!=0)
							downloaded=TRUE; //try twice
						incompl=1;
					}
					else if(result==SEGMENT_TIMEOUT)
						incompl=0;
					else if(result==TRUE){
						downloaded=TRUE;
						incompl=0;
					}
					else{ //FALSE dl'ed+incomplete if it failed
						downloaded=TRUE;
						incompl=1;
					}
					incompletes+=incompl;
					set_segment_status(xml,i,downloaded,incompl,FALSE);
					release_segment(xml,i);
				}
			}
			if(incompl!=0 || downloaded!=TRUE)
				if(!send_group_cmd(xml,ssl))
					return FALSE;
			if(!is_socket_alive(ssl->p_recv))
				return FALSE;
			retry++;
			if(downloaded)
				break;
		}
	}
	return TRUE;
}
DWORD WINAPI dl_thread(LPVOID event)
{
	ssl_context ssl;
	int port=0;
	char news_server[200];
	char user[200];
	char pass[200];
	int result=FALSE;

	memset(&ssl,0,sizeof(ssl));
	debug_pf(1,"thread started %08X\n",event);
	while(WaitForSingleObject(event,INFINITE)==WAIT_OBJECT_0){
		int doit=0;
		InterlockedIncrement(&threads_busy);
		thread_status=0;
		SetEvent(events[STATUS_EVENT]);
		news_server[0]=0;
		get_ini_str("news_server",news_server,sizeof(news_server));
		get_ini_value("news_port",&port);
		if(!can_write(ssl.p_send,1)){
			if(ssl.p_send!=0){
				disconnect(&ssl);
				ssl.p_send=0;
			}
			connect_news_server(news_server,port,&ssl);
		}
		if(can_write(ssl.p_recv,0))
		{
			char str[300];
			result=FALSE;
			user[0]=pass[0]=0;
			get_ini_str("user_name",user,sizeof(user));
			get_ini_str("password",pass,sizeof(pass));
			if(user[0]!=0){
				_snprintf(str,sizeof(str),"AUTHINFO USER %s\n",user);
				if(send_and_check(&ssl,str,sizeof(str),0)){
					_snprintf(str,sizeof(str),"AUTHINFO PASS %s\n",pass);
					if(send_and_check(&ssl,str,sizeof(str),"2xx"))
							result=TRUE;
					else{
						strlwr(str);
						if(strstr(str,"invalid"))result=FALSE;
						if(strstr(str,"already"))result=TRUE;
						if(strstr(str,"authenticated"))result=TRUE;
					}
				}
				if(result==FALSE){
					thread_status=THREAD_NC;
				}
			}
			else{
				if(check_basic_command(&ssl)) //maybe they dont need a U/P
					result=TRUE;
				else
					thread_status=THREAD_CREDS;
			}
			if(result){
				int xml=0,item=0;
				while((xml=get_xml_param(item))!=0){
					int dl=dl_file(&ssl,xml);
					item++;
					if(stop_everything()){
						thread_status=THREAD_ABORTED;
						break;
					}

					if(!is_socket_alive(ssl.p_recv)){
						debug_pf(2,"main thread loop:socket not alive\n");
						thread_status=THREAD_LOST;
						break;
					}
					if(!dl){
						if(!check_basic_command(&ssl)){
							debug_pf(2,"main thread loop:basic command failed\n");
							thread_status=THREAD_LOST;
							break;
						}
					}

				}
			}
		}
		else
			thread_status=THREAD_NC;

		disconnect(&ssl);memset(&ssl,0,sizeof(ssl));
		InterlockedDecrement(&threads_busy);
		ResetEvent(event);
	}
	return TRUE;
}

int claim_segment(int xml,int index)
{
	int id=0;
	int claimed=FALSE;
	static int count=1;
	EnterCriticalSection(&cs);
	id=get_segment_thread_id(xml,index);
	if(id==0){
		if(set_segment_thread_id(xml,index,count)){
			claimed=TRUE;
			count++;
			if(count==0)
				count=1;
		}
	}
	LeaveCriticalSection(&cs);
	return claimed;
}
int release_segment(int xml,int index)
{
	int result=FALSE;
	EnterCriticalSection(&cs);
	result=set_segment_thread_id(xml,index,0);
	LeaveCriticalSection(&cs);
	return result;
}
int update_item_status()
{
	int i,count;
	int segments_left=0;
	count=listview_get_count();
	for(i=0;i<count;i++){
		int xml;
		xml=get_xml_param(i);
		if(xml){
			int segments,incomplete,downloaded,id=0;
			segments=get_segment_count(xml);
			downloaded=get_segments_downloaded(xml);
			incomplete=get_segments_incomplete(xml);
			segments_left+=segments-downloaded;
			if(downloaded==0){
				int j;
				EnterCriticalSection(&cs);
				for(j=0;j<segments;j++){
						id=get_segment_thread_id(xml,j);
						if(id!=0)
							break;
				}
				LeaveCriticalSection(&cs);
			}
			if(downloaded>0 || id!=0){
				if(downloaded<segments){
					if(threads_busy>0 && stop_everything())
						set_item_status_by_xml(xml,"[%i/%i] shutting down%s",downloaded,segments,incomplete?" (incomplete)":"");
					else if(threads_busy==0)
						if(thread_status==THREAD_ABORTED)
							set_item_status_by_xml(xml,"[%i/%i] aborted%s",downloaded,segments,incomplete?" (incomplete)":"");
						else
							set_item_status_by_xml(xml,"[%i/%i]%s",downloaded,segments,incomplete?" (incomplete)":"");
					else
						set_item_status_by_xml(xml,"[%i/%i] in progress%s",downloaded,segments,incomplete?" (incomplete)":"");
				}
				else if(downloaded>=segments)
					set_item_status_by_xml(xml,"[%i/%i] done%s",segments-incomplete,segments,incomplete?" (incomplete)":"");
			}
			else{
				set_item_status_by_xml(xml,"[%i/%i]",downloaded,segments);
			}
			
		}
	}
	return segments_left;
}
DWORD WINAPI status_thread(LPVOID event)
{
	int retrys;
	debug_pf(1,"status thread started\n");
	while(WaitForSingleObject(event,INFINITE)==WAIT_OBJECT_0){
		retrys=0;
		debug_pf(1,"status thread start\n");
		while(TRUE){
			int segments_left=0;
			if(threads_busy==0){
				segments_left=update_item_status();
				if(!stop_everything() && segments_left>0 && retrys<MAX_RETRYS){
					trigger_threads(FALSE);
					retrys++;
				}
				else
					break;
			}
			else
				segments_left=update_item_status();

			if(!stop_everything() && segments_left>threads_busy && threads_busy<get_dl_thread_count() && retrys<MAX_RETRYS){
				trigger_threads(FALSE);
				retrys++;
			}

			update_speed_status(FALSE);
			Sleep(1000);
		};
		switch(thread_status){
		default:
		case THREAD_DONE:update_status("all done");break;
		case THREAD_ABORTED:update_status("aborted");break;
		case THREAD_LOST:update_status("connection lost!");break;
		case THREAD_NC:update_status("cant login to news server,check servername,username and password");break;
		case THREAD_CREDS:update_status("cant execute command on server,check your authentication settings");break;
		}
		update_title("uNZB");
		unset_go_button();
		debug_pf(1,"status thread reset\n");
		ResetEvent(event);
	};
	return 0;
}
int create_network_thread(HANDLE *event)
{
	DWORD id=0;
	char str[80];
	static int threadid=0;
	_snprintf(str,sizeof(str)-1,"networkthread %08X",threadid++);
	*event=CreateEvent(NULL,TRUE,FALSE,str);
	if(*event==0){
		update_status("error:unable to create event");
		return FALSE;
	}

	if(CreateThread(NULL,0,dl_thread,(LPVOID)*event,0,&id)==0){
		update_status("error:unable to create download thread");
		CloseHandle(*event);
		return FALSE;
	}
	return TRUE;
}

int create_status_thread(HANDLE *event)
{
	DWORD id=0;
	*event=CreateEvent(NULL,TRUE,FALSE,"status thread");
	if(*event==0){
		return FALSE;
	}
	if(CreateThread(NULL,0,status_thread,(LPVOID)*event,0,&id)==0){
		CloseHandle(*event);
		return FALSE;
	}
	return TRUE;
}
int create_threads(HWND hwnd)
{
	int i,threads,failed=0;
	threads=1;
	InitializeCriticalSection(&cs);
	get_ini_value("threads",&threads);
	memset(events,0,sizeof(events));
	if(threads<=0)threads=1;
	if(threads>MAX_THREADS)threads=MAX_THREADS;
	for(i=0;i<threads;i++){
		if(!create_network_thread(&events[i])){
			events[i]=0;
			failed++;
		}
	}
	if(!create_status_thread(&events[STATUS_EVENT]))
		failed++;
	if(failed>0)
		MessageBox(hwnd,"failed to create download threads","major error",MB_OK);
	return TRUE;
}
int get_dl_thread_count()
{
	int i,count=0;
	for(i=0;i<MAX_THREADS;i++){
		if(events[i]==0)
			break;
		count++;
	}
	return count;
}
int trigger_threads(int cold_start)
{
	int i;
	if(cold_start){
		update_byte_counters();
		update_speed_status(TRUE);
	}
	for(i=0;i<sizeof(events)/sizeof(HANDLE);i++){
		if(events[i]==0)
			break;
		SetEvent(events[i]);
	}
	return TRUE;
}
int any_threads_busy()
{
	return threads_busy;
}