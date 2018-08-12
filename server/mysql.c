#include"factory.h"
void mysql(char query[],char p[512])
{
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char* server="localhost";
	char* user="root";
	char* password="123456";
	char* database="ftp";
	int t;
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0))
	{
		printf("Error connecting to database:%s\n",mysql_error(conn));
	}else{
		printf("connected...\n");
	}
	t=mysql_query(conn,query);
	if(t)
	{
		printf("Error making query:%s\n",mysql_error(conn));
	}else{
		res=mysql_use_result(conn);
		if(res)
		{
			row=mysql_fetch_row(res);
			if(row==NULL)
			{
				strcpy(p,"NULL");
			}
			while(row)
				//while((row=mysql_fetch_row(res))!=NULL)
			{
				sprintf(p,"%s%s",p,row[0]);
				row=mysql_fetch_row(res);
				if(row!=NULL)
					strcat(p,"    ");
			}
		}
		mysql_free_result(res);
	}
	mysql_close(conn);
}


