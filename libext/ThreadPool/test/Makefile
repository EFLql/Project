#test for ThreadPool makefile
GTEST_DIR=../../..
LIB_DIR=../../..
CPPFLAGS=-isystem $(GTEST_DIR)/
CXXFLAGS=-std=gnu++1y -g -pthread

GTEST_HEAD=$(GTEST_DIR)/gtest/*.h $(GTEST_DIR)/gtest/internal/*.h

OBJS=../*.o $(LIB_DIR)/libext/EventBaseManger.o \
	 $(LIB_DIR)/libext/asyn/EventBase.o \
	 $(LIB_DIR)/libext/asyn/EventHandler.o \
	 $(LIB_DIR)/libext/FileUtil.o

PRO_NAME=ThreadPoolTest

$(PRO_NAME): $(OBJS) $(PRO_NAME).o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $(PRO_NAME).o $(OBJS) $(LIB_DIR)/libgtest.a $(LIB_DIR)/libgtest_main.a \
		-lpthread -lglog -levent
$(PRO_NAME).o: $(PRO_NAME).cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $(PRO_NAME).cpp

clean:
	rm *.o $(PRO_NAME)
