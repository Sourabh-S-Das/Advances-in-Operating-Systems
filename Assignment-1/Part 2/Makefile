file = partb_1_20CS30051_20CS30061

obj-m+=$(file).o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load:
	sudo insmod $(file).ko

unload:
	sudo rmmod $(file)
	
test_1:
	gcc ./test_files/test_1.c && ./a.out

test_2:
	gcc ./test_files/test_2.c && ./a.out
	
test_3:
	gcc ./test_files/test_3.c && ./a.out

test_conc:
	g++ ./test_files/concurrency_test.cpp && ./a.out /proc/partb_1_20CS30051_20CS30061

test_edge:
	g++ ./test_files/edge_case_test.cpp && ./a.out /proc/partb_1_20CS30051_20CS30061

test_queue:
	g++ ./test_files/queue_correctness_test.cpp && ./a.out /proc/partb_1_20CS30051_20CS30061
	
test_all:
	make test_conc
	make test_edge
	make test_queue
	
clean_test:
	rm ./a.out
