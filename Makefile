
all:
	g++ -o oom_adj_daemon -Wall -ansi -pedantic -std=c++11 exec-notify.c

