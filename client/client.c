#include"func.h"

int main(int argc,char* argv[])
{
	if(argc!=3)
	{
		printf("./client IP PORT\n");
		return -1;
	}
	int sfd;
	sfd=socket(AF_INET,SOCK_STREAM,0);
	if(-1==sfd)
	{
		perror("socket");
		return -1;
	}
	struct sockaddr_in ser;
	int ret;
	bzero(&ser,sizeof(ser));
	ser.sin_family=AF_INET;
	ser.sin_port=htons(atoi(argv[2]));
	ser.sin_addr.s_addr=inet_addr(argv[1]);
	ret=connect(sfd,(struct sockaddr*)&ser,sizeof(struct sockaddr));
	if(-1==ret)
	{
		perror("connect");
		return -1;
	}
	char buf[1000]={0};
	char* passwd;
	int len;
	int dir;
	int load=0;
	char opt;
	static int times=0;
	action act;
flag:	printf("1.login\n2.register\n0.quit\n");
		scanf("%c",&opt);
		while(1)
		{
			if(opt=='0')
			{
				exit(0);
			}else if(opt!='1'&&opt!='2'){
				printf("1.login\n2.register\n0.quit\n");
				scanf("%c",&opt);
				scanf("%c",&opt);
			}else if(opt=='\n'){
			}else{
				break;
			}
		}
		act.action_flag=opt-'0';
		if(act.action_flag==1)//登陆
		{
			load=0;
			printf("login:");
			scanf("%s",act.buf);
			send(sfd,&act,sizeof(act),0);
			memset(act.buf,0,sizeof(act.buf));
			recv(sfd,&act,sizeof(act),0);//salt
input:		passwd=getpass("password:");
			strcpy(act.buf,crypt(passwd,act.buf));
			send(sfd,&act,sizeof(act),0);
			recv(sfd,&load,sizeof(int),0);
			if(load)
			{
				printf("welcome.\n");
				printf("Enter -help for help\n");
			}else{
				printf("error password.\n");
				times++;
				if(times<3){	
					goto input;
				}else{
					printf("password error too much times.\n");
					exit(0);
				}
			}
		}else if(act.action_flag==2){
			printf("User Name:");
			memset(act.buf,0,sizeof(act.buf));
			scanf("%s",act.buf);
			send(sfd,&act,sizeof(act),0);
			memset(act.buf,0,sizeof(act.buf));
			recv(sfd,&act,sizeof(act),0);
			while(act.action_flag==-1)
			{
				printf("User Name Exists.\n");
				act.action_flag=2;
				printf("User Name:");
				memset(act.buf,0,sizeof(act.buf));
				scanf("%s",act.buf);
				send(sfd,&act,sizeof(act),0);
				memset(act.buf,0,sizeof(act.buf));
				recv(sfd,&act,sizeof(act),0);	
			}
			act.action_flag=-1;
			passwd=getpass("Password:");
			//memset(act.buf,0,sizeof(act.buf));
			strcpy(act.buf,crypt(passwd,act.buf));
			send(sfd,&act,sizeof(act),0);
			recv(sfd,&load,sizeof(int),0);
			if(load==1)
			{
				printf("register success.\n");
				goto flag;
			}
		}
		char cmd[128];
		char obj[128];
		while(1)
		{
			memset(buf,0,sizeof(buf));
			memset(cmd,0,sizeof(cmd));
			memset(act.buf,0,sizeof(act.buf));
			read(0,act.buf,sizeof(buf));
			for(int i=0,j=0,k=0;i<strlen(act.buf);i++)
			{
				while(act.buf[i]!=' ')
				{
					cmd[j++]=act.buf[i];
					i++;
					if(i>=strlen(act.buf))
						break;
				}
				while(act.buf[i]==' ')
					i++;
				while(act.buf[i]!=' ')
				{
					buf[k++]=act.buf[i];
					i++;
					if(i>=strlen(act.buf))
						break;
				}
				if(strlen(buf)!=0)
					buf[k-1]='\0';
				else
					cmd[j-1]='\0';
			}
			//printf("cmd=%s\n",cmd);
			//printf("buf=%s\n",buf);
			memset(act.buf,0,sizeof(act.buf));
			if(strcmp(cmd,"gets")==0)
			{
				act.action_flag=3;
				strcpy(act.buf,buf);
				send(sfd,&act,sizeof(act),0);
				recv(sfd,&act,sizeof(act),0);
				if(act.action_flag==-1)
				{
					printf("%s\n",act.buf);
				}else{
					dir=FindFile(buf);
					struct stat buffer;
					train t;
					bzero(&t,0);
					if(dir==0)
					{
						int fd=open(buf,O_CREAT,0666);
						close(fd);
					}
					stat(buf,&buffer);
					memcpy(t.buf,&buffer.st_size,sizeof(off_t));
					t.len=sizeof(off_t);
					send(sfd,&t,4+t.len,0);
					recv_file(sfd,buf,buffer.st_size);
					recv(sfd,&act,sizeof(act),0);
					printf("%s\n",act.buf);
				}
			}else if(strcmp(cmd,"puts")==0){
				act.action_flag=4;
				dir=FindFile(buf);
				if(dir==0)
				{
					printf("No Such File.dir=%d\n",dir);
				}else if(dir==1){
					strcpy(act.buf,buf);
					strcpy(cmd,buf);
					send(sfd,&act,sizeof(act),0);//fname
					memset(act.buf,0,sizeof(act.buf));
					MD5SUM(buf,act.buf);
					printf("md5=%s\n",act.buf);
					send(sfd,&act,sizeof(act),0);//md5
					int samefilename;
					recv(sfd,&samefilename,4,0);
					if(samefilename==0)
					{
						int len,exist;
						recv(sfd,&exist,sizeof(int),0);
						if(exist==0)
						{
							struct stat st;
							int fd=open(cmd,O_RDWR);
							fstat(fd,&st);
							len=st.st_size;
							send(sfd,&len,sizeof(int),0);
							printf("filesize=%d\n",len);
							if(len<=1024*1024*100)
							{
								trans_file(sfd,cmd);
							}else{
								char* p=mmap(NULL,len,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
								int total=0;
								while(total<len)
								{
									strncpy(buf,p+total,1000);
									send_n(sfd,buf,1000);
									total+=1000;
								}
								munmap(p,len);
								close(fd);
							}
						}else if(exist==1){
							printf("秒传\n");
						}
					}	
					memset(act.buf,0,sizeof(act.buf));
					recv(sfd,&act,sizeof(act),0);
					printf("%s\n",act.buf);
				}
			}else if(strcmp(cmd,"ls")==0){
				act.action_flag=5;
				strcpy(act.buf,buf);
				send(sfd,&act,sizeof(act),0);
				memset(act.buf,0,sizeof(act.buf));
				recv(sfd,&act,sizeof(act),0);
				printf("%s\n",act.buf);
			}else if(strcmp(cmd,"rm")==0){
				act.action_flag=6;
				strcpy(act.buf,buf);
				send(sfd,&act,sizeof(act),0);
				memset(act.buf,0,sizeof(act.buf));
				recv(sfd,&act,sizeof(act),0);
				printf("%s\n",act.buf);
			}else if(strcmp(cmd,"pwd")==0){
				act.action_flag=7;
				strcpy(act.buf,buf);
				send(sfd,&act,sizeof(act),0);
				memset(act.buf,0,sizeof(buf));
				recv(sfd,&act,sizeof(act),0);
				printf("%s\n",act.buf);
			}else if(strcmp(cmd,"cd")==0){
				act.action_flag=8;
				strcpy(act.buf,buf);
				send(sfd,&act,sizeof(act),0);
				memset(act.buf,0,sizeof(buf));
				recv(sfd,&act,sizeof(act),0);
				printf("%s\n",act.buf);
			}else if(strcmp(cmd,"exit")==0){
				exit(0);
			}else if(strcmp(cmd,"mkdir")==0){
				act.action_flag=9;
				strcpy(act.buf,buf);
				send(sfd,&act,sizeof(act),0);
				memset(act.buf,0,sizeof(act.buf));
				recv(sfd,&act,sizeof(act),0);
				printf("%s\n",act.buf);
			}else if(strcmp(cmd,"-help")==0){
				printf("1.cd 进入对应目录\n2.ls 列出相应目录文件\n3.puts 将本地文件上传服务器\n4.gets 文件名 下载服务器文件到本地\n5.rm 删除服务器上文件\n6.pwd 显示目前所在路径\n7.mkdir 目录名 创建新目录\n8.rmdir 目录名 删除目录所有文件\n9.exit 退出\n");
			}else if(strcmp(cmd,"rmdir")==0){
				act.action_flag=10;
				strcpy(act.buf,buf);
				send(sfd,&act,sizeof(act),0);
				memset(act.buf,0,sizeof(act.buf));
				recv(sfd,&act,sizeof(act),0);
				printf("%s\n",act.buf);
			}
		}
		return 0;
}
