client:
	gcc -c client.c -lcrypt
	gcc -c ftp.c
	gcc -c md5sum.c -lcrypto
	gcc -o client *.o -lcrypt -lcrypto
clean:
	rm *.o client
