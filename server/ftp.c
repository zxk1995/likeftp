#include"factory.h"
void factory_init(factory *p,int th_num,int capacity)
{
	bzero(p,sizeof(factory));//清空结构体
	p->pth=(pthread_t*)calloc(th_num,sizeof(pthread_t));
	pthread_cond_init(&p->cond,NULL);
	p->pthread_num=th_num;
	p->que.que_capacity=capacity;
	pthread_mutex_init(&p->que.que_mutex,NULL);
}
void factory_start(factory *p)
{
	int i;
	if(!p->start_flag)
	{
		for(i=0;i<p->pthread_num;i++)
		{
			pthread_create(p->pth+i,NULL,threadfunc,p);
		}
		p->start_flag=1;//启动成功，修改启动标志
	}
}
void que_insert(pque_t pq,pnode_t pnew)
{
	if(!pq->que_head)
	{
		pq->que_head=pnew;
		pq->que_tail=pnew;
	}else{
		pq->que_tail->pnext=pnew;
		pq->que_tail=pnew;
	}
	pq->que_size++;
}
void que_get(pque_t pq,pnode_t* pcur)
{
	*pcur=pq->que_head;
	pq->que_head=pq->que_head->pnext;
	pq->que_size--;
}
void* threadfunc(void* p1)
{
	factory* f=(factory*)p1;
	pque_t pq=&f->que;
	pnode_t pcur=NULL;
	char flag;
	int new_fd,len;
	char buf[128]={0};
	char p[512];
	char s[128];
	time_t tm;
	while(1)
	{
		pthread_mutex_lock(&pq->que_mutex);
		if(!pq->que_size)
		{
			pthread_cond_wait(&f->cond,&pq->que_mutex);
		}
		que_get(pq,&pcur);
		pthread_mutex_unlock(&pq->que_mutex);
		if(pcur)
		{
			action act;	
			message m;
			int load=0,procode=0,id=0,ret;
			char route[512]="/";
			char res[512]={0};
			while(1)
			{
				new_fd=pcur->new_fd;
				printf("recv new cmd\n");
				ret=recv(new_fd,&act,sizeof(act),0);
				if(0==ret)
					break;
				tm=time(NULL);
				bzero(s,0);
				sprintf(s,"%suserID=%d act.flag=%d act.buf=%s\t %s",s,id,act.action_flag,act.buf,4+ctime(&tm));
				int fd=open("log.txt",O_RDWR|O_CREAT|O_APPEND,0600);
				write(fd,s,strlen(s));
				close(fd);
				if(act.action_flag==1)
				{
					strcpy(p,"select salt from User where Name='");
					sprintf(p,"%s%s'",p,act.buf);
					printf("%s\n",p);
					mysql(p,res);
					printf("salt=%s\n",res);
					memset(buf,0,sizeof(buf));
					strcpy(buf,res);
					strcpy(p,"select userID from User where Name='");
					sprintf(p,"%s%s'",p,act.buf);
					printf("%s\n",p);
					memset(res,0,sizeof(res));
					mysql(p,res);
					id=atoi(res);
					printf("userID=%d\n",id);
					strcpy(act.buf,buf);
					send(new_fd,&act,sizeof(act),0);
passwd:			memset(act.buf,0,sizeof(act.buf));
				ret=recv(new_fd,&act,sizeof(act),0);
				if(ret==0)
					break;
				strcpy(p,"select ciphertext from User where ciphertext='");
				sprintf(p,"%s%s'",p,act.buf);
				memset(res,0,sizeof(res));
				printf("%s\n",p);
				mysql(p,res);
				memset(buf,0,sizeof(buf));
				strcpy(buf,res);
				printf("ciphertext=%s\n",buf);
				printf("client_ciphertext=%s\n",act.buf);
				if(strcmp(act.buf,buf)==0)
				{
					load=1;
					procode=0;
				}
				send(new_fd,&load,sizeof(int),0);
				if(load==0)
					goto passwd;
				}else if(act.action_flag==2){
					while(act.action_flag==2)
					{
						strcpy(p,"select Name from User where Name='");
						sprintf(p,"%s%s'",p,act.buf);
						memset(res,0,sizeof(res));
						printf("%s\n",p);
						mysql(p,res);
						strcpy(p,act.buf);//name
						if(strcmp(res,"NULL")==0)
						{
							memset(res,0,sizeof(res));
							strcpy(res,"$6$");
							get_rand_str(res,8);//salt
							memset(act.buf,0,sizeof(act.buf));
							strcpy(act.buf,res);
							printf("rand str=%s\n",act.buf);
						}else{
							act.action_flag=-1;
						}
						send(new_fd,&act,sizeof(act),0);
						recv(new_fd,&act,sizeof(act),0);
					}
					printf("client=%s\n",act.buf);
					char temp[]="insert into User(Name,salt,ciphertext) value('";
					sprintf(temp,"%s%s','%s','%s')",temp,p,res,act.buf);
					mysql(temp,res);
					strcpy(res,"select userID from User where Name='");
					sprintf(res,"%s%s'",res,p);
					printf("%s\n",res);
					memset(res,0,sizeof(res));
					mysql(p,res);
					id=atoi(res);
					printf("userID=%d\n",id);
					procode=0;
					load=1;
					send(new_fd,&load,sizeof(int),0);
				}else if(act.action_flag==3){
					strcpy(p,"select md5 from VirtualDirectory where filename='");
					sprintf(p,"%s%s' and be_id=%d and type='-'",p,act.buf,id);
					printf("%s\n",p);
					memset(res,0,sizeof(res));
					mysql(p,res);
					if(strcmp(res,"NULL")==0)
					{
						act.action_flag=-1;
						strcpy(act.buf,"No Such File.");
						send(new_fd,&act,sizeof(act),0);
					}else{
						send(new_fd,&act,sizeof(act),0);
						memset(act.buf,0,sizeof(act.buf));
						int len;
						recv(new_fd,&len,sizeof(int),0);
						off_t off_set;
						recv(new_fd,&off_set,len,0);
						trans_file(new_fd,res,off_set);
						strcpy(act.buf,"download success.");
						send(new_fd,&act,sizeof(act),0);
					}
				}else if(act.action_flag==4){
					char prodir[32]={0};
					int i,j,k;
					for(i=0,j=0;i<strlen(route);i++)
					{
						if(route[i]=='/')
							j++;
					}
					if(j==1)
					{
						strcpy(prodir,"/");
					}else{
						for(i=0,k=0;i<strlen(route);i++)
						{
							if(route[i]!='/'&&(k==j-1))
								sprintf(prodir,"%s%c",prodir,route[i]);
							if(route[i]=='/')
								k++;
						}
					}
					strcpy(p,act.buf);//filename
					memset(act.buf,0,sizeof(act.buf));
					recv(new_fd,&act,sizeof(act),0);//md5
					char temp[]="select md5 from VirtualDirectory where md5='";//exist?
					sprintf(temp,"%s%s'",temp,act.buf);
					printf("%s\n",temp);
					memset(res,0,sizeof(res));
					mysql(temp,res);
					//have same filename?
					strcpy(temp,"select filename from VirtualDirectory where type='-' and be_id=");
					sprintf(temp,"%s%d and filename='%s' and procode=%d and prodir='%s'",temp,id,p,procode,prodir);//prodir?
					printf("%s\n",temp);
					char res2[512]={0};
					int samefilename;
					mysql(temp,res2);
					if(strcmp(res2,"NULL")==0)
					{
						samefilename=0;
					}else{
						samefilename=1;
					}
					send(new_fd,&samefilename,4,0);
					if(samefilename==0)
					{
						strcpy(temp,"insert into VirtualDirectory(procode,filename,type,be_id,md5,prodir) value(");
						sprintf(temp,"%s%d,'%s','-',%d,'%s','%s')",temp,procode,p,id,act.buf,prodir);
						printf("%s\n",temp);
						char res1[512]={0};
						mysql(temp,res1);
						int len,exist;
						if(strcmp(res,"NULL")!=0)
						{
							exist=1;
							send(new_fd,&exist,sizeof(int),0);
						}else{
							exist=0;
							send(new_fd,&exist,sizeof(int),0);
							recv(new_fd,&len,sizeof(int),0);
							printf("recv file,filesize=%d\n",len);
							if(len<=1024*1024*100)
							{
								printf("recvfile,not mmap\n");
								recv_file(new_fd,act.buf);
							}else{
								printf("recvfile,mmap\n");
								strcpy(res,route);
								int fd=open(act.buf,O_RDWR|O_CREAT,0666);
								ftruncate(fd,len);
								char* p=mmap(NULL,len,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
								int total=0;
								while(total<len)
								{
									recv_n(new_fd,buf,1000);
									strncpy(p+total,buf,1000);
									total+=1000;
								}
								munmap(p,len);
								close(fd);
								strcpy(route,res);
							}
						}
						memset(act.buf,0,sizeof(act.buf));
						strcpy(act.buf,"puts file success.");
					}else{
						strcpy(act.buf,"Have Same Name File Or File Exist.");
					}
					send(new_fd,&act,sizeof(act),0);
				}else if(act.action_flag==5){
					char prodir[32]={0};
					int i,j,k;
					for(i=0,j=0;i<strlen(route);i++)
					{
						if(route[i]=='/')
							j++;
					}
					if(j==1)
					{
						strcpy(prodir,"/");
					}else{
						for(i=0,k=0;i<strlen(route);i++)
						{
							if(route[i]!='/'&&(k==j-1))
								sprintf(prodir,"%s%c",prodir,route[i]);
							if(route[i]=='/')
								k++;
						}
					}
					strcpy(p,"select filename from VirtualDirectory where procode=");
					sprintf(p,"%s%d and be_id=%d and prodir='%s'",p,procode,id,prodir);
					printf("%s\n",p);
					memset(res,0,sizeof(res));
					mysql(p,res);
					strcpy(act.buf,res);
					send(new_fd,&act,sizeof(act),0);
				}else if(act.action_flag==6){
					char prodir[32]={0};
					int i,j,k;
					for(i=0,j=0;i<strlen(route);i++)
					{
						if(route[i]=='/')
							j++;
					}
					if(j==1)
					{
						strcpy(prodir,"/");
					}else{
						for(i=0,k=0;i<strlen(route);i++)
						{
							if(route[i]!='/'&&(k==j-1))
								sprintf(prodir,"%s%c",prodir,route[i]);
							if(route[i]=='/')
								k++;
						}
					}
					strcpy(p,"select filename from VirtualDirectory where filename='");
					sprintf(p,"%s%s' and be_id=%d and type='-' and prodir='%s' and procode=%d",p,act.buf,id,prodir,procode);
					printf("%s\n",p);
					memset(res,0,sizeof(res));
					mysql(p,res);
					if(strcmp(res,"NULL")==0)
					{
						strcpy(act.buf,"No Such File.");
					}else{
						strcpy(p,"delete from VirtualDirectory where filename='");
						sprintf(p,"%s%s' and be_id=%d and type='-' and procode=%d and prodir='%s'",p,act.buf,id,procode,prodir);
						printf("%s\n",p);
						mysql(p,res);
						strcpy(act.buf,"rm success.");
					}
					send(new_fd,&act,sizeof(act),0);
				}else if(act.action_flag==7){
					//memset(act.buf,0,sizeof(act.buf));
					strcpy(act.buf,route);
					send(new_fd,&act,sizeof(act),0);
				}else if(act.action_flag==8){
					if(strcmp(act.buf,".")==0)
					{
						strcpy(act.buf,"cd success.");
					}else if(strcmp(act.buf,"..")==0){
						if(strcmp(route,"/")==0)
						{
							strcpy(act.buf,route);
						}else{
							for(int i=0,j=0;i<strlen(route);i++)
							{
								if(route[i]=='/')
									j++;
								if(j==procode)
								{
									route[i+1]='\0';
									break;
								}
							}
							procode--;
						}
						strcpy(act.buf,"cd success.");
					}else{
						strcpy(p,"select filename from VirtualDirectory where filename='");
						sprintf(p,"%s%s' and be_id=%d and procode=%d and type='d'",p,act.buf,id,procode);
						printf("%s\n",p);
						memset(res,0,sizeof(res));
						mysql(p,res);
						if(strcmp(res,"NULL")==0)
						{
							strcpy(act.buf,"cd failed.");
						}else{
							strcpy(act.buf,"cd success.");
							sprintf(route,"%s%s%s",route,res,"/");
							procode++;
						}
					}
					send(new_fd,&act,sizeof(act),0);
				}else if(act.action_flag==9){
					char prodir[32]={0};
					int i,j,k;
					for(i=0,j=0;i<strlen(route);i++)
					{
						if(route[i]=='/')
							j++;
					}
					if(j==1)
					{
						strcpy(prodir,"/");
					}else{
						for(i=0,k=0;i<strlen(route);i++)
						{
							if(route[i]!='/'&&(k==j-1))
								sprintf(prodir,"%s%c",prodir,route[i]);
							if(route[i]=='/')
								k++;
						}
					}
					strcpy(p,"insert into VirtualDirectory(procode,filename,type,be_id,prodir) value(");
					sprintf(p,"%s%d,'%s','d',%d,'%s')",p,procode,act.buf,id,prodir);
					printf("%s\n",p);
					mysql(p,res);
					strcpy(act.buf,"mkdir success.");
					send(new_fd,&act,sizeof(act),0);
				}else if(act.action_flag==10){
					char prodir[32];
					int i,j,k;
					for(i=0,j=0;i<strlen(route);i++)
					{
						if(route[i]=='/')
							j++;
					}
					if(j==1)
					{
						strcpy(prodir,"/");
					}else{
						for(i=0,k=0;i<strlen(route);i++)
						{
							if(route[i]!='/'&&(k==j-1))
								sprintf(prodir,"%s%c",prodir,route[i]);
							if(route[i]=='/')
								k++;
						}
					}
					strcpy(p,"select filename from VirtualDirectory where procode=");
					sprintf(p,"%s%d and be_id=%d and prodir='%s'",p,procode+1,id,act.buf);
					printf("%s\n",p);
					memset(res,0,sizeof(res));
					mysql(p,res);
					if(strcmp(res,"NULL")==0)
					{
						strcpy(p,"delete from VirtualDirectory where filename='");
						sprintf(p,"%s%s' and be_id=%d and type='d' and procode=%d and prodir='%s'",p,act.buf,id,procode,prodir);
						printf("%s\n",p);
						mysql(p,res);
						strcpy(act.buf,"rmdir success.");
					}else{
						strcpy(act.buf,"rmdir fail.");
					}
					send(new_fd,&act,sizeof(act),0);
				}
			}
		}
		free(pcur);
		pcur=NULL;
	}
}
void get_rand_str(char s[],int num)
{
	char* str="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz,./;\"'<>?";
	int i,lstr;
	char ss[2]={0};
	lstr=strlen(str);
	srand((unsigned int)time((time_t*)NULL));
	for(i=1;i<=num;i++)
	{
		sprintf(ss,"%c",str[(rand()%lstr)]);
		strcat(s,ss);
	}
}
void trans_file(int sfd,char* filename,off_t off_set)
{
	printf("off_set=%f\n",(float)off_set);
	train t;
	t.len=strlen(filename);
	strcpy(t.buf,filename);
	send(sfd,&t,4+t.len,0);
	struct stat buf;
	stat(t.buf,&buf);
	memcpy(t.buf,&buf.st_size,sizeof(off_t));
	t.len=sizeof(off_t);
	send_n(sfd,(char*)&t,4+t.len);
	int fd,ret;
	fd=open(filename,O_RDWR);
	lseek(fd,(off_t)off_set-1,SEEK_SET);
	while((t.len=read(fd,t.buf,sizeof(t.buf)))>0)
	{
		ret=send_n(sfd,(char*)&t,4+t.len);
		if(-1==ret)
		{
			return;
		}
	}
	send(sfd,&t,4+t.len,0);
	printf("send file success.\n");
	close(fd);
}
void recv_file(int sfd,char* filename)
{
	//	printf("begin recv.\n");
	int len;
	char buf[1024]={0};
	//	recv(sfd,&len,sizeof(int),0);
	//	recv(sfd,buf,len,0);//filename
	//	printf("filename=%s\n",buf);
	//	recv(sfd,&len,sizeof(int),0);
	//	off_t file_size;
	//	recv(sfd,&file_size,len,0);//filesize
	//	printf("filesize=%d\n",file_size);
	int fd=open(filename,O_RDWR|O_CREAT,0666);
	puts(filename);
	if(-1==fd)
	{
		perror("open");
		return;
	}
	//	time_t before,now;
	//	now=time(NULL);
	//	before=now;
	//	float real_size=0;
	while(1)
	{
		recv_n(sfd,(char*)&len,sizeof(int));
		if(len>0)
		{
			//	printf("recv_n..\n");
			recv_n(sfd,buf,len);
			write(fd,buf,len);
			//	real_size=real_size+len;
			//	now=time(NULL);
			//	if(now-before>=1)
			//	{
			//		printf("\r%5.2f%s",real_size/file_size*100,"%");
			//		fflush(stdout);
			//		before=now;
			//
		}else{
			//	printf("\r100.00%s\n","%");
			// 	printf("file recv success.\n");
			break;
		}
		}
		close(fd);
		return;
	}
	int send_n(int sfd,char* p,int len)
	{
		int ret;
		int total=0;
		while(total<len)
		{
			ret=send(sfd,p+total,len-total,0);
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
			if(ret==0)
			{
				return 0;
			}
			total=total+ret;
		}
		return 0;
	}
	int FindFile(char s[])
	{
		char path[]=".";
		DIR* dir=opendir(path);
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
				return 1;
		}
		return 0;
	}
