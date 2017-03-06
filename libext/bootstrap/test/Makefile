MYPATH=../../..
GTEST_DIR=../../..
LIB_DIR=../../..
HOME=$(MYPATH)/libext
CXXFLAGS+= -std=gnu++1y -g -pthread
CPPFLAGS+= -isystem $(GTEST_DIR)/

PRO_NAME=ServerBootstrapTest
#PRO_NAME=test
OBJS=$(HOME)/SocketAddr.o $(HOME)/EventBaseManger.o $(HOME)/FileUtil.o $(HOME)/bootstrap/*.o \
	 $(HOME)/asyn/*.o $(HOME)/ThreadPool/*.o 

GTEST_HEAD=$(GTEST_DIR)/gtest/*.h $(GTEST_DIR)/gtest/internal/*.h

$(PRO_NAME): $(OBJS) $(PRO_NAME).o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $(PRO_NAME).o $(OBJS) $(LIB_DIR)/libgtest.a $(LIB_DIR)/libgtest_main.a -lpthread -levent -lglog

$(PRO_NAME).o: $(GTEST_HEAD) $(PRO_NAME).cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $(PRO_NAME).cpp

clean:
	rm *.o $(PRO_NAME)
