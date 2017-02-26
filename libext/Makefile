vpath %.h ../
MACRO=DEBUGALL
CFLAGS+= -std=gnu++1y -g -w -D$(MACRO)
SOURCES=$(wildcard *.cpp)
OBJS :=$(patsubst %.cpp, %.o,$(SOURCES))
SPATH=../
CC=gcc
entry: $(OBJS)
	@echo "sources files:" $(SOURCES)
	@echo "object files:" $(OBJS)
	$(CC) $(OBJS) -I$(SPATH) -D $(MACRO) $(CFLAGS) -o entry
%.o: %.cpp
	$(CC) -o $@ -c $< $(CFLAGS) -I$(SPATH)
sinclude $(SOURCES:.cpp=.d)
%.d: %.cpp
	@echo "create depend"
	$(CC) -MM $(CFLAGS) $< > $@.$$$$;\
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;\
	$(RM) $@.$$$$
	
clean:
	rm -rf $(OBJS)
	rm -f entry
