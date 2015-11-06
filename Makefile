
all:
	flex -c++ config_parser.lex
	g++ -c -Wall -ansi -pedantic -std=c++11 exec-notify.c
	g++ -o oom_adj_daemon -Wall -ansi -pedantic -std=c++11 exec-notify.o

