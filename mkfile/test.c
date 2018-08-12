#include"func.h"
int main()
{
	int fd=open("testfile",O_RDWR|O_CREAT,0666);
	ftruncate(fd,1024*1024*500);
	return 0;
}
