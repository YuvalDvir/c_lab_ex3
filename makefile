FLAGS = -Wall -Wextra

.PHONY:	clean all test ex3_lb

all: ex3_lb 

clean:
	rm -rf *.o ex3_lb 

ex3_lb:	load_balancer.o
	gcc load_balancer.o -o ex3_lb 

load_balancer.o: 	load_balancer.c
			gcc $(FLAGS) -g -c load_balancer.c

test:	all
	~nimrodav/socket_ex/test.sh
