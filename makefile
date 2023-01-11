compile: client2 server2

client2: basic_client.o pipe_networking.o
	gcc -o cliento basic_client.o pipe_networking.o

client: cliento
	./cliento

server2: basic_server.o pipe_networking.o
	gcc -o servero basic_server.o pipe_networking.o

server: servero
	./servero

basic_client.o: basic_client.c pipe_networking.h
	gcc -c basic_client.c

basic_server.o: basic_server.c pipe_networking.h
	gcc -c basic_server.c

pipe_networking.o: pipe_networking.c pipe_networking.h
	gcc -c pipe_networking.c

clean:
	rm *.o
	rm *~
