FROM ubuntu

RUN apt-get update -y
RUN apt-get upgrade -y
RUN apt-get install build-essential -y

COPY server.c server.c

RUN gcc server.c -o server
ENTRYPOINT ["./server"]