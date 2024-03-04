## Advances in Operating Systems Design
### Assignment 2

Sourabh Soumyakanta Das - 20CS30051\
Shiladitya De - 20CS30061

### Part A
To run part A, in one terminal execute the following commands:
```
cd partA
make object
make load
```
To run the server, in another terminal, run the following command:
```
make server
```
To run the client, run the following:
```
make client
```

To see the debugging statements, run the following command:
```
sudo cat /sys/kernel/debug/tracing/trace_pipe
```
### Part B
To run part B, run the following commands:
```
cd partB
make object
make load
```

To run the servers, run the following commands (in three separate terminals):
```
make server1
make server2
make server3
```

To run the client, run the following command:
```
make client
```
To see the debugging statements, run the following command:
```
sudo cat /sys/kernel/debug/tracing/trace_pipe
```

