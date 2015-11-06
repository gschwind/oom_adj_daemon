
all:
	g++ -c -Wall -std=c++11 exec-notify.cxx
	g++ -o oom_adj_daemon -Wall -std=c++11 exec-notify.o -lboost_regex

clean:
	rm -r *.o
	rm oom_adj_daemon
