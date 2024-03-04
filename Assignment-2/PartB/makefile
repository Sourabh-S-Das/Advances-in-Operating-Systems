object:
	clang -g -O2 -target bpf -D__TARGET_ARCH_x86_64 -I . -c partB.c -o bpf.object.o

load: loader
	sudo ./loader

loader: loader.c
	gcc loader.c -o loader -lbpf

server: server.c
	gcc server.c -o server
	
server1: server
	./server 8081

server2: server
	./server 8082
	
server3: server
	./server 8083

client: client.c
	gcc client.c -o client && ./client 8080

clean:
	rm bpf.object.o loader server client
