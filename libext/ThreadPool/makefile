test : ThreadFactory.o Thread.o Task.o ThreadPoolExecutor.o test.o
	g++ -std=gnu++11 -g -o test Task.o ThreadFactory.o Thread.o ThreadPoolExecutor.o test.o  -lpthread

#Queue.o : Queue.cpp typedefine.h Task.h Queue.h
#	g++ -std=gnu++11 -c Queue.cpp

ThreadFactory.o : ThreadFactory.cpp typedefine.h ThreadFactory.h
	g++ -std=gnu++11 -g -c ThreadFactory.cpp

Thread.o : Thread.cpp typedefine.h Task.h Queue.h Thread.h 
	g++ -std=gnu++11 -g -c Thread.cpp

Task.o : Task.cpp typedefine.h Task.h
	g++ -std=gnu++11 -g -c Task.cpp

ThreadPoolExecutor.o : ThreadPoolExecutor.cpp typedefine.h Queue.h Task.h ThreadPoolExecutor.h
	g++ -std=gnu++11 -g -c ThreadPoolExecutor.cpp

test.o : test.cpp typedefine.h ThreadPoolExecutor.h 
	g++ -std=gnu++11 -g -c test.cpp
