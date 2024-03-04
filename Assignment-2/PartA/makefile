object:
	clang -g -O2 -target bpf -D__TARGET_ARCH_x86_64 -I . -c partA.c -o bpf.object.o

load: loader
	sudo ./loader

loader:
	gcc loader.c -o loader -lbpf

server:
	gcc server.c -o server && ./server 8080

client:
	gcc client.c -o client && ./client 8080

clean:
	rm bpf.object.o loader server client
