#ifndef __FACTORY_H__
#define __FACTORY_H__
#include "head.h"
#include "work_que.h"
typedef struct
{
	pthread_t *pth;//存储线程id的起始指针
	pthread_cond_t cond;
	que_t que;//队列
	int pthread_num;//线程数
	short start_flag;//启动标志,0代表未启动，1代表启动
}factory;
void factory_init(factory*,int,int);
void factory_start(factory*);
void* threadfunc(void*);
void trans_file(int,char*,off_t);
void recv_file(int,char*);
int send_n(int,char*,int);
typedef struct{
	int len;
	char buf[1000];
}train;
typedef struct act{
	int action_flag;
	char buf[256];
}action;
int recv_n(int,char*,int);
void mysql(char*,char*);
void get_rand_str(char*,int);
void write_log(char*,message*);
int FindFile(char*);
void MD5SUM(char*,char*);
#endif
