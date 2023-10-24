FROM ubuntu

RUN apt-get update -y
RUN apt-get upgrade -y
RUN apt-get install build-essential -y

COPY client.c client.c

RUN gcc client.c -o client
ENTRYPOINT ["./client"]