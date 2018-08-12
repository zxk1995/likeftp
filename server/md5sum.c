#include"factory.h"
void MD5SUM(char* filename,char* buf)  
{  
	MD5_CTX ctx;  
	unsigned char outmd[16];  
	char buffer[1024];  
	int len=0;  
	int i;  
	FILE * fp=NULL;  
	memset(outmd,0,sizeof(outmd));  
	//memset(filename,0,sizeof(filename));  
	memset(buffer,0,sizeof(buffer));  
	fp=fopen(filename,"rb");  
	if(fp==NULL)  
	{  
		printf("Can't open file\n");  
		return;  
	}  
	MD5_Init(&ctx);  
	while((len=fread(buffer,1,1024,fp))>0)  
	{  
		MD5_Update(&ctx,buffer,len);  
		memset(buffer,0,sizeof(buffer));  
	}  
	MD5_Final(outmd,&ctx);  
	for(i=0;i<16;i<i++)  
	{  
		sprintf(buf,"%s%02x",buf,outmd[i]);  
	}  
	printf("%s\n",buf);  
	fclose(fp);
	return;  
}  


