#include"func.h"
void recv_file(int sfd,char* filename,off_t off_set)
{
	int len;
	char buf[1000]={0};
	recv(sfd,&len,sizeof(int),0);
	recv(sfd,buf,len,0);//filename
	recv(sfd,&len,sizeof(int),0);
	off_t file_size;
	recv(sfd,&file_size,len,0);
	printf("filesize=%f\n",(float)file_size);
	int fd=open(filename,O_RDWR|O_CREAT,0666);
	lseek(fd,(off_t)off_set-1,SEEK_SET);
	if(-1==fd)
	{
		perror("open");
		return;
	}
	time_t before,now;
	now=time(NULL);
	before=now;
	float real_size=(float)off_set;
	printf("realsize=%f\n",real_size);
	while(1)
	{
		recv_n(sfd,(char*)&len,sizeof(int));
		if(len>0)
		{
			recv_n(sfd,buf,len);
			write(fd,buf,len);
			real_size=real_size+len;
			now=time(NULL);
			if(now-before>=1)
			{ printf("\r%5.2f%s",real_size/file_size*100,"%");
				fflush(stdout);
				before=now;
			}
		}else{
			printf("\r100.00%s\n","%");
			break;
		}
	}
	close(fd);
	return;
}
void trans_file(int sfd,char* filename)
{
	train t;
	//	t.len=strlen(filename);
	//	strcpy(t.buf,filename);
	//	puts(filename);
	//	send(sfd,&t,4+t.len,0);//filename
	//	struct stat buf;
	//	stat(filename,&buf);
	//	printf("buf.st_size=%d",buf.st_size);
	//	memcpy(t.buf,&buf.st_size,sizeof(off_t));
	//	t.len=sizeof(off_t);
	//	printf("filesize=%s\n",t.buf);
	//	send_n(sfd,(char*)&t,4+t.len);//filesize
	int fd;
	printf("filename=%s\n",filename);
	fd=open(filename,O_RDWR);
	int ret;
	while((t.len=read(fd,t.buf,sizeof(t.buf)))>0)
	{
		ret=send_n(sfd,(char*)&t,4+t.len);
		if(-1==ret)
		{
			return;
		}
	}
	send(sfd,&t,4+t.len,0);
	close(fd);
	return;
}
int FindFile(char s[])
{
	char path[]=".";
	DIR *dir=opendir(path);
	if(NULL==dir)
	{
		perror("opendir");
		return -1;
	}
	struct dirent* p=readdir(dir);
	while((p=readdir(dir))!=NULL)
	{
		if(strcmp(p->d_name,".")==0||strcmp(p->d_name,"..")==0)
			continue;
		if(strcmp(s,p->d_name)==0)
		{
			return 1;
		}
	}
	return 0;
}
int send_n(int sfd,char* p,int len)
{
	int ret;
	int total=0;
	while(total<len)
	{
		ret=send(sfd,p+total,len-total,0);
		if(ret==-1)
		{
			return -1;
		}
		total=total+ret;
	}
	return 0;
}
int recv_n(int sfd,char* p,int len)
{
	int ret;
	int total=0;
	while(total<len)
	{
		ret=recv(sfd,p+total,len-total,0);
		if(0==ret)
		{
			return -1;
		}
		total=total+ret;
	}
	return 0;
}

