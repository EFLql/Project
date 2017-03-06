GTEST_DIR= ../..
LIB_DIR = ../..
USER_DIR = ./
#传给预处理器
CPPFLAGS += -isystem $(GTEST_DIR)/
#传给编译器
CXXFLAGS += -std=gnu++1y -g -pthread
GTEST_HEAD=$(GTEST_DIR)/gtest/*.h \
		   $(GTEST_DIR)/gtest/internal/*.h
PRO_NAME=SocketAddrTest
SRCS=$(PRO_NAME).cpp
OBJS=../SocketAddr.o
$(PRO_NAME): ../SocketAddr.o $(PRO_NAME).o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS)  -o $@ $(PRO_NAME).o $(OBJS) -lpthread $(LIB_DIR)/libgtest.a $(LIB_DIR)/libgtest_main.a
$(PRO_NAME).o: $(SRCS) $(GTEST_HEAD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(PRO_NAME).cpp -o $@ 

clean:
	rm *.o $(PRO_NAME) 
