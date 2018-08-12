#ifndef __FUNC_H__
#define __FUNC_H__
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <signal.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <sys/uio.h>
#include<shadow.h>
#include<crypt.h>
#include<openssl/md5.h>
typedef struct{
	int len;
	char buf[1000];
}train;
typedef struct mysql_reserch{
	int load;
	char buf[1000];
	char id[32];
	char procode[32];
}msql,*pmsql;
typedef struct act{
	int action_flag;
	char buf[256];
}action;
void trans_file(int,char*);
int send_n(int,char*,int);
int recv_n(int,char*,int);
int FindFile(char*);
void recv_file(int,char*,off_t);
void MD5SUM(char*,char*);
#endif
