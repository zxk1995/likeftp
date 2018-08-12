server:
	gcc -c mysql.c -I /usr/include/mysql -lmysqlclient
	gcc -c ftp.c -pthread
	gcc -c server.c -pthread
	gcc -c md5sum.c -lcrypto
	gcc -o server *.o -I /usr/include/mysql/ -lmysqlclient -pthread -lcrypto
clean:
	rm *.o server
