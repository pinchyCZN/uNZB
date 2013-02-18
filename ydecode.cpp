#include <iostream>
//#include <stdlib.h>
//#include <stdio.h>
/*
=ybegin part=1 line=128 size=422143 name=RiffTrax_Laser_Mission.nzb
=ypart begin=1 end=249600
..=. for network stream
=yend size=249600 part=1 pcrc32=d3e48f4c
CRLF.CRLF  is end of message
*/
extern "C" int _fseeki64(FILE *stream,__int64 offset,int origin);
extern "C" __int64 _ftelli64(FILE *stream);
extern "C" debug_pf(int,char*,...);
#define TRUE 1
#define FALSE 0
#define CR '\r'
#define LF '\n'
#define EQU '='

#define MAX_PATH 260
enum{
	START_LINE=0,
	START,
	CHECK_Y,
	CHECK_Y_ANY,
	CHECK_DOT,
	CHECK_ENDM,
	NO_MATCH,
	NEED_MORE,
	EOL,
	YBEGIN_PARAMS,
	YBEGIN_PART,
	YBEGIN_LINE,
	YBEGIN_SIZE,
	YBEGIN_NAME,
	YPART_PARAMS,
	YPART_BEGIN,
	YPART_END,
	YEND_PARAMS,
	YEND_SIZE,
	YEND_PART,
	YEND_PCRC32,
	YEND_CRC32,
	DECODE,
	DECODE_ESCAPE_CHAR,
	DECODE_STALL,
};
typedef struct{
	char *keyword;
	int nextstate;
	//int(*func)(BYTE *,int);
}STATE;

static STATE states[]={
	{"=ybegin ",YBEGIN_PARAMS},
	{"part=",YBEGIN_PART},
	{"line=",YBEGIN_LINE},
	{"size=",YBEGIN_SIZE},
	{"name=",YBEGIN_NAME},
	{"=ypart ",YPART_PARAMS},
	{"begin=",YPART_BEGIN},
	{"end=",YPART_END},
	{"=yend ",YEND_PARAMS},
	{"pcrc32=",YEND_PCRC32},
	{"crc32=",YEND_CRC32},

};
static const int crc_tab[256] =
{
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
	0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
	0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
	0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
	0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
	0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
	0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
	0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
	0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
	0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
	0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
	0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
	0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
	0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
	0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
	0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};
class ydecoder
{
public:
int part,line,size,begin,end,yend_size,yend_part,pcrc32,crc32,have_pcrc32,have_crc32;
int outpos,name_pos,line_pos,yend_reached,file_saved,end_message;
char name[MAX_PATH];
int decode_state;
char parse[80];
int parse_pos;

~ydecoder(){
}
ydecoder(){
	part=line=size=begin=end=yend_size=yend_part=pcrc32=crc32=have_pcrc32=have_crc32=0;
	decode_state=START_LINE;
	name_pos=outpos=parse_pos=0;
	end_message=file_saved=yend_reached=0;
	memset(name,0,sizeof(name));
}

int calc_crc(char *buf,int len)
{
	unsigned int i,crc_val=0xFFFFFFFF;
	unsigned int ch1,ch2,cc;
	  /* X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+X^0 */
	for(i=0;i<len;i++){
		cc=buf[i]&0xFF;
		ch1=(crc_val ^ cc) & 0xFF;
		ch1=crc_tab[ch1];
		ch2=(crc_val>>8)&0xFFFFFF;
		crc_val=ch1 ^ ch2;
	}
	return crc_val^0xFFFFFFFF;
}
int calc_crc_file(char *fname)
{
	unsigned int crc_val=0xFFFFFFFF;
	unsigned int ch1,ch2,cc;
	__int64 i,index,fsize;
	char *buf;
	FILE *f;
	buf=(char*)malloc(0x100000);
	if(buf==0)
		return 0;
	f=fopen(fname,"rb");
	if(f==0){
		free(buf);
		return 0;
	}
	_fseeki64(f,0,SEEK_END);
	fsize=_ftelli64(f);
	_fseeki64(f,0,SEEK_SET);
	index=0;
	for(i=0;i<fsize;i++){
		if(index==0)
			fread(buf,1,0x100000,f);
		cc=buf[index]&0xFF;
		ch1=(crc_val ^ cc) & 0xFF;
		ch1=crc_tab[ch1];
		ch2=(crc_val>>8)&0xFFFFFF;
		crc_val=ch1 ^ ch2;
		index++;
		index%=0x100000;
	}
	free(buf);
	fclose(f);
	return crc_val^0xFFFFFFFF;
}
int is_number(char a)
{
	if(a>='0' && a<='9')
		return TRUE;
	else
		return FALSE;
}
int is_whitespace(char a)
{
	if(a==' ' || a=='\t')
		return TRUE;
	else
		return FALSE;
}
int is_eol(char a)
{
	if(a==CR || a==LF)
		return TRUE;
	else
		return FALSE;
}
int trim_trailing_space(char *str)
{
	int i,len=strlen(str);
	for(i=len-1;i>=0;i--){
		if(is_whitespace(str[i]))
			str[i]=0;
		else
			break;
	}
	return TRUE;
}
int handle_number(char a,int *val,int *decode_state,int parent_state)
{
	if(is_number(a))
		*val=(*val)*10+(a-'0');
	else if(is_whitespace(a))
		*decode_state=parent_state;
	else if(is_eol(a))
		*decode_state=START_LINE;
	else
		*decode_state=START;
	return TRUE;
}
int handle_str(char a,char *str,int *str_pos,int *decode_state,int parent_state)
{
	if(name_pos>=sizeof(name)-1){
		str[*str_pos]=0;
		*decode_state=parent_state;
	}
	else if(a==CR || a==LF || a==0){
		str[(*str_pos)++]=0;
		*decode_state=START_LINE;
	}
	else
		str[(*str_pos)++]=a;
	return TRUE;
}
int handle_crc(char a,int *val,int *decode_state,int parent_state)
{
	if(a>='0' && a<='9')
		*val=((*val)<<4)+(a-'0');
	else if(a>='A' && a<='F')
		*val=((*val)<<4)+(a-'A'+10);
	else if(a>='a' && a<='f')
		*val=((*val)<<4)+(a-'a'+10);
	else if(is_whitespace(a))
		*decode_state=parent_state;
	else if(is_eol(a))
		*decode_state=START_LINE;
	else
		*decode_state=START;
	return TRUE;
}
int find_match(char *buf,int len)
{
	int i,j;
	int match=NO_MATCH;
	for(i=0;i<sizeof(states)/sizeof(STATE);i++){
		int max=strlen(states[i].keyword);
		for(j=0;j<max && j<len;j++){
			if(is_eol(buf[j]))
				return EOL;
			if(buf[j]!=states[i].keyword[j])
				break;
		}
		if(j==len)
			match=NEED_MORE;
		if(j==max && j==len){
			match=states[i].nextstate;
			break;
		}
	}
	return match;
}
int is_valid_special_char(char a)
{
	if(a=='@' || a=='J' || a=='M' || a=='}') //0,cr,lf,=
		return TRUE;
	else
		return FALSE;
}
int is_ready_decode()
{
	if(line!=0 && size!=0 && name[0]!=0)
		return TRUE;
	else
		return FALSE;
}
int decode_char(unsigned char a,char *dst,int *dst_pos,int dlen)
{
	if(yend_reached){
		debug_pf(4,"yend reached\n");
		return 0;
	}
	if(*dst_pos>=dlen){
		debug_pf(4,"size exceeded\n");
		return 0;
	}
	dst[(*dst_pos)++]=a-42;
	line_pos++;
	return TRUE;
}
int decode_esc_char(unsigned char a,char *dst,int *dst_pos,int dlen)
{
	return decode_char(a-64,dst,dst_pos,dlen);
}
int ydecode(char *src,int slen,char *dst,int dlen)
{
	int i,result;
	unsigned char a;
	static int *var=0;
	for(i=0;i<slen;i++){
		a=src[i];
		switch(decode_state){
		default:
		case START:
		case START_LINE:
			line_pos=0;
			if(a=='='){
				parse_pos=0;
				decode_state=CHECK_Y;
				parse[parse_pos++]=a;
				line_pos++;
			}
			else if(a=='.'){
				decode_state=CHECK_DOT;
			}
			else if(a==CR || a==LF)// || a==0)
				;
			else if(is_ready_decode()){
				decode_state=DECODE;
				if(a!='.') //skip double dots
					decode_char(a,dst,&outpos,dlen);
			}
			else
				decode_state=START;
			break;
		case CHECK_DOT:
			if(a=='.'){
				decode_char(a,dst,&outpos,dlen);
				decode_state=DECODE;
			}
			else if(a==CR)
				decode_state=CHECK_ENDM;
			else
				decode_state=START;
			break;
		case CHECK_ENDM:
			if(a==LF)
				end_message=TRUE;
			decode_state=START;
			break;
		case CHECK_Y:
			if(a=='y'){
				decode_state=CHECK_Y_ANY;
				parse[parse_pos++]=a;
			}
			else{
				if(is_ready_decode()){
					decode_esc_char(a,dst,&outpos,dlen);
					decode_state=DECODE;
				}else{
					decode_state=START;
				//	printf("not ready to decode yet, pos=%i outpos=%i\n",i,outpos);
				}
			}
			break;
		case CHECK_Y_ANY:
			parse[parse_pos++]=a;
			result=find_match(parse,parse_pos);
			switch(result){
			case YBEGIN_PARAMS:
			case YPART_PARAMS:
			case YEND_PARAMS:
				decode_state=result;
				parse_pos=0;
				break;
			default:
				parse_pos=0;
				break;
			case EOL:
				decode_state=START_LINE;
				break;
			case NEED_MORE:
				break;
			case NO_MATCH:
				parse_pos=0;
				break;
			}
			break;
		case DECODE:
			//if(outpos==0x5a30-1)
			//	i=i;
			if(!is_ready_decode())
				decode_state=START;
			else if(a=='='){
				decode_state=DECODE_ESCAPE_CHAR;
				line_pos++;
				if(line_pos>=line)
					decode_state=DECODE_STALL;
			}
			else if(a==CR || a==LF){
				decode_state=START_LINE;
			}
			else{
				if(line_pos>=line)
					decode_state=START;
				decode_char(a,dst,&outpos,dlen);
				if(line_pos>=line)
					decode_state=START;
			}
			break;
		case DECODE_ESCAPE_CHAR:
			decode_state=DECODE;
			//if(is_valid_special_char(a))
			if(line_pos<line){
				decode_esc_char(a,dst,&outpos,dlen);
				if(line_pos>=line)
					decode_state=START_LINE;
			}
			else{
				if(line_pos>=line)
					decode_state=DECODE_STALL;
				else{
					debug_pf(4,"invalid escape code %02X\n",a);
					decode_state=START;
				}
			}
			break;
		case DECODE_STALL: //edge case for at end of line
			if(a==CR || a==LF)
				line_pos=0;
			else if(a!='y'){
				decode_esc_char(a,dst,&outpos,dlen);
				decode_state=DECODE;
				line_pos++;
			}
			else{
				debug_pf(4,"invalid escape code %02X\n",a);
				decode_state=START;
			}

			break;
		/*--------------*/
		case YBEGIN_PARAMS:
			end_message=file_saved=yend_reached=FALSE;
			parse[parse_pos++]=a;
			result=find_match(parse,parse_pos);
			switch(result){
			default:
				decode_state=START;
				break;
			case NO_MATCH:
				parse_pos=0;
				break;
			case NEED_MORE:
				break;
			case EOL:
				decode_state=START_LINE;
				break;
			case YBEGIN_PART:
				part=0;
				decode_state=YBEGIN_PART;
				break;
			case YBEGIN_LINE:
				line=0;
				decode_state=YBEGIN_LINE;
				break;
			case YBEGIN_SIZE:
				size=0;
				decode_state=YBEGIN_SIZE;
				break;
			case YBEGIN_NAME:
				memset(name,0,sizeof(name));
				name_pos=0;
				decode_state=YBEGIN_NAME;
				break;
			}
			break;
		case YBEGIN_PART:
			handle_number(src[i],&part,&decode_state,YBEGIN_PARAMS);
			parse_pos=0;
			break;
		case YBEGIN_LINE:
			handle_number(src[i],&line,&decode_state,YBEGIN_PARAMS);
			parse_pos=0;
			break;
		case YBEGIN_SIZE:
			handle_number(src[i],&size,&decode_state,YBEGIN_PARAMS);
			parse_pos=0;
			break;
		case YBEGIN_NAME:
			handle_str(src[i],name,&name_pos,&decode_state,YBEGIN_PARAMS);
			if(decode_state!=YBEGIN_NAME)
				trim_trailing_space(name);
			parse_pos=0;
			break;
		/*--------------*/
		case YPART_PARAMS:
			parse[parse_pos++]=a;
			result=find_match(parse,parse_pos);
			switch(result){
			default:
				decode_state=START;
				break;
			case NO_MATCH:
				parse_pos=0;
				break;
			case NEED_MORE:
				break;
			case EOL:
				decode_state=START_LINE;
				break;
			case YPART_BEGIN:
				begin=0;
				decode_state=YPART_BEGIN;
				break;
			case YPART_END:
				end=0;
				decode_state=YPART_END;
				break;
			}
			break;
		case YPART_BEGIN:
			handle_number(src[i],&begin,&decode_state,YPART_PARAMS);
			parse_pos=0;
			break;
		case YPART_END:
			handle_number(src[i],&end,&decode_state,YPART_PARAMS);
			parse_pos=0;
			break;
		/*--------------*/
		case YEND_PARAMS:
			yend_reached=TRUE;
			parse[parse_pos++]=a;
			result=find_match(parse,parse_pos);
			switch(result){
			default:
				decode_state=START;
				break;
			case NO_MATCH:
				parse_pos=0;
				break;
			case NEED_MORE:
				break;
			case EOL:
				decode_state=START_LINE;
				break;
			case YBEGIN_PART:
				yend_part=0;
				decode_state=YEND_PART;
				break;
			case YBEGIN_SIZE:
				yend_size=0;
				decode_state=YEND_SIZE;
				break;
			case YEND_CRC32:
				crc32=0;
				decode_state=YEND_CRC32;
				break;
			case YEND_PCRC32:
				pcrc32=0;
				decode_state=YEND_PCRC32;
				break;
			}
			break;
		case YEND_CRC32:
			handle_crc(src[i],&crc32,&decode_state,YEND_PARAMS);
			have_crc32=TRUE;
			parse_pos=0;
			break;
		case YEND_PCRC32:
			handle_crc(src[i],&pcrc32,&decode_state,YEND_PARAMS);
			have_pcrc32=TRUE;
			parse_pos=0;
			break;
		case YEND_PART:
			handle_number(src[i],&yend_part,&decode_state,YEND_PARAMS);
			parse_pos=0;
			break;
		case YEND_SIZE:
			handle_number(src[i],&yend_size,&decode_state,YEND_PARAMS);
			parse_pos=0;
			break;
		}
		if(parse_pos>=sizeof(parse) || parse_pos>256)
			parse_pos=0;
	}
	return TRUE;
}
int yenc_verify_file(char *buf,int bsize,char *path)
{
	int len=bsize<outpos?bsize:outpos;
	int crc,result=TRUE;
/* not thread safe yet
	if(have_crc32){
		char fname[MAX_PATH];
		memset(fname,0,sizeof(fname));
		_snprintf(fname,sizeof(fname)-1,"%s\\%s",path,name);
		crc=calc_crc_file(fname);
		if(crc!=crc32){
			debug_pf(4,"error file crc:crc32:%08X yenc crc32:%08X\n",crc,crc32);
			result=FALSE;
		}
	}
*/
	if(have_pcrc32 || have_crc32){
		crc=calc_crc(buf,len);
		//for now dont bother with full file crc unless it was just one segment with no part crc32
		if((have_pcrc32 && crc!=pcrc32) || (have_crc32 && crc!=crc32)){
			debug_pf(4,"error segment:calc crc32:%08X yenc pcrc32:%08X crc32:%08X\n",crc,pcrc32,crc32);
			result=FALSE;
		}
	}
	return result;
}

int save_data(char *buf,int len,int pos,char *name,char *path)
{
	FILE *f;
	char fpath[MAX_PATH+1];
	memset(fpath,0,sizeof(fpath));
	_snprintf(fpath,sizeof(fpath)-1,"%s\\%s",path,name);
	f=fopen(fpath,"rb+");
	if(f==0)
		f=fopen(fpath,"wb+");
	if(f!=0){
		fseek(f,pos,SEEK_SET);
		fwrite(buf,1,len,f);
		fclose(f);
		return TRUE;
	}
	return FALSE;
}
int yec_save_file(char *dst,char *path)
{
	if(outpos>0 && begin>0 && end>0 && end>begin){
		int truesize=(end-begin+1),true_begin=begin-1;
		if(outpos>=truesize){
			if(save_data(dst,truesize,true_begin,name,path)){
				file_saved=TRUE;
				debug_pf(3,"saved file %s\nsize=%i offset=%i\n",name,truesize,true_begin);
				if(outpos>truesize)
					debug_pf(3,"warning outpos was greater than end-begin %i %i\n",outpos,truesize);
			}
			else
				debug_pf(3,"unable to save file %s size=%i\n",name,truesize);
		}
		else
			end=end;
	}
	else if(outpos>0 && size>0){
		int true_begin=begin-1;
		if(true_begin<0)
			true_begin=0;
		if(save_data(dst,outpos,true_begin,name,path))
			file_saved=FALSE;
		debug_pf(3,"warning begin and end not valid so saving as is\n");
	}
	return file_saved;
}


int test_ydecode()
{
	char *buf1,*buf2;
	FILE *f=0;
	int len;
//	f=fopen("test3.bin","rb");
	if(f==0){
		//f=fopen("b:\\Ronaldinho Soccer 64 (Brazil) (Pirate) - CARROT.nfo","rb");
		f=fopen("b:\\test4.bin","rb");
	}
	if(f!=0){
		fseek(f,0,SEEK_END);
		len=ftell(f);
		fseek(f,0,SEEK_SET);
		buf1=(char*)malloc(0x100000);
		buf2=(char*)malloc(0x100000);
		fread(buf1,1,len,f);
		fclose(f);
		memset(buf2,0,sizeof(buf2));
//		ydec_init();
		
		ydecode(buf1,len,buf2,0x100000);
		yenc_verify_file(buf2,outpos,"");
		yec_save_file(buf2,"b:\\");
		/*
		fout=fopen("decoded.bin","wb");
		if(fout!=0){
			fwrite(buf2,1,outpos,fout);
			fclose(fout);
		}
		*/
		//outpos=len;
		//yenc_verify_file(buf1,len);

		free(buf1);
		free(buf2);
	}
	return TRUE;
}
}; //end ydecoder class

extern "C" int ydec_new()
{
	ydecoder *ys=new ydecoder();
	return (int)ys;
}
extern "C" int ydec_delete(ydecoder *ys)
{
	delete ys;
	return TRUE;
}
extern "C" int ydecode(ydecoder *ys,char *src,int slen,char *dst,int dlen)
{
	return ys->ydecode(src,slen,dst,dlen);
}

extern "C" int is_end_message(ydecoder *ys)
{
	return ys->end_message;
}
extern "C" int is_yend_reached(ydecoder *ys)
{
	return ys->yend_reached;
}
extern "C" int yec_save_file(ydecoder *ys,char *dst,char *path)
{
	return ys->yec_save_file(dst,path);
}
extern "C" int yenc_verify_file(ydecoder *ys,char *buf,int bsize,char *path)
{
	return ys->yenc_verify_file(buf,bsize,path);
}