#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "tinyxml.h"
#include "Commctrl.h"

extern "C"{
int update_byte_counters();
int listview_get_count();
int get_listview_file(char *path,int len);
int get_item_dl_path(int xml,char *str,int len);
int get_xml_param(int item);
int find_valid_path(char *path);
int get_ini_file_path(char **path);
int	get_ini_str(char *key,char *str,int size);
int write_ini_str(char *key,char *str);
int add_item(const char *fname,
			 const char *dlpath,
			 const char *status,
			 const char *group,
			 const char *poster,
			 const char *date,
			 const char *subject,
			 int xml
			 );
}
int extract_fname(const char *subject,char *fname,int max)
{
	int i,len,index=0;
	int start=FALSE;
	len=strlen(subject);
	for(i=0;i<len;i++){
		if(index>=max){
			index=0;
			break;
		}
		if(start){
			if(subject[i]=='\"'){
				start=FALSE;
				fname[index++]=0;
				break;
			}
			fname[index++]=subject[i];
		}
		else if(subject[i]=='\"')
			start=TRUE;
	}
	return index>0;
}
extern "C" int free_xml_node(int p)
{
	TiXmlNode *xml=(TiXmlNode *)p;
	delete xml;
	return TRUE;
}
extern "C" int get_segment_count(int xml)
{
	TiXmlNode *node=(TiXmlNode *)xml;
	TiXmlNode *child=0,*parent=node->FirstChild("segments");
	int segments=0;
	if(parent){
		while(child=parent->IterateChildren(child)){
			segments++;
		}
	}
	return segments;
}
extern "C" int unset_incomplete_status(int xml)
{
	int count=0;
	TiXmlNode *node=(TiXmlNode *)xml;
	TiXmlNode *child=0,*parent=node->FirstChild("segments");
	if(parent){
		while(child=parent->IterateChildren(child)){
			unsigned int downloaded=0,incomplete=0;
			TiXmlElement *seg=child->ToElement();
			seg->QueryUnsignedAttribute("downloaded",&downloaded);
			seg->QueryUnsignedAttribute("incomplete",&incomplete);
			if(downloaded!=0 && incomplete!=0){
				seg->RemoveAttribute("incomplete");
				seg->RemoveAttribute("downloaded");
				count++;
			}
		}
	}
	return count;
}
extern "C" int set_segment_status(int xml,int index,int downloaded,int incomplete,int all)
{
	TiXmlNode *node=(TiXmlNode *)xml;
	TiXmlNode *child=0,*parent=node->FirstChild("segments");
	int segments=0;
	if(parent){
		while(child=parent->IterateChildren(child)){
			TiXmlElement *seg=child->ToElement();
			if(all || segments==index){
				if(downloaded)
					seg->SetAttribute("downloaded",1);
				else
					seg->RemoveAttribute("downloaded");
				if(incomplete)
					seg->SetAttribute("incomplete",1);
				else
					seg->RemoveAttribute("incomplete");
			}
			segments++;
		}
	}
	return FALSE;
}

extern "C" int get_segment_info(int xml,int index,const char **name,unsigned int *incomplete,unsigned int *downloaded)
{
	TiXmlNode *node=(TiXmlNode *)xml;
	TiXmlNode *child=0,*parent=node->FirstChild("segments");
	int segments=0;
	if(parent){
		while(child=parent->IterateChildren(child)){
			TiXmlElement *seg=child->ToElement();
			if(segments==index){
				if(name!=0)*name=seg->GetText();
				if(incomplete!=0){
					unsigned int inc=0;
					seg->QueryUnsignedAttribute("incomplete",&inc);
					*incomplete+=inc;
				}
				if(downloaded!=0){
					unsigned int d=0;
					seg->QueryUnsignedAttribute("downloaded",&d);
					*downloaded+=d;
				}
				return TRUE;
			}
			segments++;
		}
	}
	return FALSE;
}
extern "C" int get_segment_thread_id(int xml,int index)
{
	TiXmlNode *node=(TiXmlNode *)xml;
	TiXmlNode *child=0,*parent=node->FirstChild("segments");
	int segments=0;
	if(parent){
		while(child=parent->IterateChildren(child)){
			TiXmlElement *seg=child->ToElement();
			if(segments==index){
				unsigned int thread_id=0;
				seg->QueryUnsignedAttribute("thread_id",&thread_id);
				if(thread_id!=0)
					return thread_id;
			}
			segments++;
		}
	}
	return 0;
}
extern "C" int set_segment_thread_id(int xml,int index,int thread_id)
{
	TiXmlNode *node=(TiXmlNode *)xml;
	TiXmlNode *child=0,*parent=node->FirstChild("segments");
	int segments=0;
	if(parent){
		while(child=parent->IterateChildren(child)){
			TiXmlElement *seg=child->ToElement();
			if(segments==index){
				if(thread_id==0)
					seg->RemoveAttribute("thread_id");
				else
					seg->SetAttribute("thread_id",thread_id);
				return TRUE;
			}
			segments++;
		}
	}
	return FALSE;
}
extern "C" int remove_all_thread_id(TiXmlNode *file)
{
	TiXmlNode *child=0,*parent=file->FirstChild("segments");
	int segments=0;
	if(parent){
		while(child=parent->IterateChildren(child)){
			TiXmlElement *seg=child->ToElement();
			seg->RemoveAttribute("thread_id");
		}
	}
	return TRUE;
}
extern "C" int is_segment_done(int xml,int index)
{
	unsigned int downloaded=0;
	get_segment_info(xml,index,0,0,&downloaded);
	return downloaded;
}
extern "C" int is_file_done(int xml)
{
	int i,downloaded=0,segments=get_segment_count(xml);
	for(i=0;i<segments;i++){
		unsigned int d=0;
		get_segment_info(xml,i,NULL,NULL,&d);
		if(d!=0)
			downloaded++;
	}
	if(downloaded==segments)
		return TRUE;
	else
		return FALSE;
}
extern "C" int get_group_name(int xml,const char **group)
{
	TiXmlNode *node=(TiXmlNode *)xml;
	node=node->FirstChild("groups");
	if(node){
		node=node->FirstChild("group");
		if(node){
			*group=node->ToElement()->GetText();
			return TRUE;
		}
	}
	return FALSE;
}

extern "C" int get_segments_downloaded(int xml)
{
	int i,segments=0,done=0;
	segments=get_segment_count(xml);
	for(i=0;i<segments;i++){
		unsigned int d=0;
		get_segment_info(xml,i,0,0,&d);
		if(d)done++;
	}
	return done;
}
extern "C" int get_segments_incomplete(int xml)
{
	int i,segments=0,incompletes=0;
	segments=get_segment_count(xml);
	for(i=0;i<segments;i++){
		unsigned int inc=0;
		get_segment_info(xml,i,0,&inc,0);
		if(inc!=0)
			incompletes++;
	}
	return incompletes;
}
extern "C" int get_total_bytes(unsigned __int64 *total,unsigned __int64 *downloaded)
{
	int i,count=0;
	*total=0;*downloaded=0;
	count=listview_get_count();
	for(i=0;i<count;i++){
		TiXmlNode *xml=(TiXmlNode *)get_xml_param(i);
		if(xml){
			xml=xml->FirstChild("segments");
			if(xml){
				TiXmlNode *child=0;
				while(child=xml->IterateChildren(child)){
					unsigned int bytes=0,d=0;
					TiXmlElement *s=child->ToElement();
					s->QueryUnsignedAttribute("bytes",&bytes);
					*total+=bytes;
					s->QueryUnsignedAttribute("downloaded",&d);
					if(d)
						*downloaded+=bytes;
				}
			}
		}
	}
	return TRUE;
}
extern "C" int load_nzb_file(char *nzb_name)
{
	TiXmlDocument doc(nzb_name);
	if(doc.LoadFile()){
		TiXmlNode *node=0;
		int count=0;
		node=doc.FirstChild("nzb");
		if(node){
			for(node=node->FirstChild("file");node;node=node->NextSibling("file")){
				TiXmlElement *file=0;
				count++;
				file=node->ToElement();
				if(file){
					const char *subj=file->Attribute("subject");
					const char *pstr=file->Attribute("poster");
					const char *date=file->Attribute("date");
					const char *group=0;
					get_group_name((int)file,&group);
					if(group==0)
						continue; //dont even try if you cant get this
					char dlpath[MAX_PATH]={0};
					char fname[MAX_PATH]={0};
					if(!extract_fname(subj,fname,sizeof(fname))){
						_snprintf(fname,sizeof(fname),"unable to extract filename,will try to get it when downloaded");
					}
					char status[100]={0};
					int segments=get_segment_count((int)file);
					int incomplete=get_segments_incomplete((int)file);
					int done=get_segments_downloaded((int)file);
					_snprintf(status,sizeof(status)-1,"[%i/%i]%s%s",done-incomplete,segments,done>=segments?" done":"",incomplete?" (incomplete)":"");
					const char *p=file->Attribute("downloadpath");
					if(p)
						strncpy(dlpath,p,sizeof(dlpath));
					else
						get_ini_str("default_dir1",dlpath,sizeof(dlpath));
					remove_all_thread_id(file);
					TiXmlNode *xml=file->Clone();
					add_item(fname,dlpath,status,group,pstr,date,subj,(int)xml);
					
				}
			}
			update_byte_counters();
		}
	}
	else{
		printf( "Could not load file %s. Error='%s'\n",nzb_name,doc.ErrorDesc());
	}
	return 0;	
}
extern "C" int handle_file_drop(HDROP hdrop)
{
	int i,count;
	char fname[MAX_PATH];
	char ext[_MAX_EXT];
	count=DragQueryFile(hdrop,-1,fname,sizeof(fname));
	for(i=0;i<count;i++){
		DragQueryFile(hdrop,i,fname,sizeof(fname));
		ext[0]=0;
		_splitpath(fname,0,0,0,ext);
		strupr(ext);
		if(strcmp(ext,".NZB")==0){
			load_nzb_file(fname);
		}
	}
	if(count>0){
		write_ini_str("last_nzb_location",fname);
		DragFinish(hdrop);
	}
	return 0;
}

extern "C" int save_listview()
{
	int i,count=0;
	count=listview_get_count();
	TiXmlDocument doc;
	TiXmlDeclaration *decl=new TiXmlDeclaration("1.0","","");
	doc.LinkEndChild(decl);
	TiXmlElement *nzb=new TiXmlElement("nzb");
	for(i=0;i<count;i++){
		TiXmlNode *xml=(TiXmlNode *)get_xml_param(i);
		if(xml){
			char path[MAX_PATH];
			TiXmlNode *node=xml->Clone();
			TiXmlElement *file=node->ToElement();
			path[0]=0;
			get_item_dl_path((int)xml,path,sizeof(path));
			if(path[0]!=0)
				file->SetAttribute("downloadpath",path);
			nzb->LinkEndChild(file);
		}
	}
	doc.LinkEndChild(nzb);
	char path[MAX_PATH];
	path[0]=0;
	if(get_listview_file(path,sizeof(path))){
		doc.SaveFile(path);
		return TRUE;
	}
	return FALSE;
}
