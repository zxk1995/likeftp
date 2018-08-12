#include"factory.h"
int pfds[2];
void sig_exit(int signum)
{
	write(pfds[1],"exit",4);
}
int main(int argc,char* argv[])
{
	if(argc!=5)
	{
		printf("./serverIP PORT THREAD_NUM CAPACITY");
		return -1;
	}
	int th_num=atoi(argv[3]);
	int capacity=atoi(argv[4]);
	factory f;
	factory_init(&f,th_num,capacity);
	factory_start(&f);
	int sfd,ret;
	sfd=socket(AF_INET,SOCK_STREAM,0);
	if(-1==sfd)
	{
		perror("socket");
		return -1;
	}
	int reuse=1;
	ret=setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(int));
	if(-1==ret)
	{
		perror("setsockopt");
		return -1;
	}
	struct sockaddr_in ser,client;
	bzero(&ser,sizeof(ser));
	ser.sin_family=AF_INET;
	ser.sin_port=htons(atoi(argv[2]));
	ser.sin_addr.s_addr=inet_addr(argv[1]);
	ret=bind(sfd,(struct sockaddr*)&ser,sizeof(struct sockaddr));
	if(-1==ret)
	{
		perror("bind");
		return -1;
	}
	int epfd=epoll_create(1);
	pipe(pfds);
	struct epoll_event event,*evs;
	evs=(struct epoll_event*)malloc(2);
	event.events=EPOLLIN;
	event.data.fd=sfd;
	epoll_ctl(epfd,EPOLL_CTL_ADD,sfd,&event);
	event.events=EPOLLIN;
	event.data.fd=pfds[0];
	epoll_ctl(epfd,EPOLL_CTL_ADD,pfds[0],&event);
	signal(SIGINT,sig_exit);
	int i,j;
	listen(sfd,capacity);
	int new_fd;
	char buf[512]={0};
	time_t tm;
	socklen_t len;
	pnode_t pnew;
	pque_t pq=&f.que;
	int rnum;
	while(1)
	{
		rnum=epoll_wait(epfd,evs,2,-1);
		for(i=0;i<rnum;i++)
		{
			if(sfd==evs[i].data.fd)
			{
			len=sizeof(struct sockaddr);
			new_fd=accept(sfd,(struct sockaddr*)&client,&len);
			int fd=open("log.txt",O_RDWR|O_CREAT|O_APPEND,0600);
			tm=time(NULL);
			bzero(buf,0);
			sprintf(buf,"%s%s %d %s",buf,inet_ntoa(client.sin_addr),ntohs(client.sin_port),4+ctime(&tm));
			write(fd,buf,strlen(buf));
			printf("client ip=%s,port=%d\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port));
			close(fd);
			pnew=(pnode_t)calloc(1,sizeof(node_t));
			pnew->new_fd=new_fd;
			pthread_mutex_lock(&pq->que_mutex);
			que_insert(pq,pnew);
			pthread_mutex_unlock(&pq->que_mutex);
			pthread_cond_signal(&f.cond);
			}
			if(pfds[0]==evs[i].data.fd)
			{
				close(sfd);
				for(i=0;i<th_num;i++)
				{
					printf("pthread %d exit.\n",i+1);
					pthread_cancel(f.pth[i]);
				}
			//	for(i=0;i<th_num;i++)
			//	{
			//		pthread_join(f.pth[i],NULL);
			//	}
				printf("main therad exit.\n");
				return 0;
			}
		}
	}
}
