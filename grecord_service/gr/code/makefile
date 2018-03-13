#CC = /usr/bin/g++-4.8
CC = g++

CFLAGS = -std=c++14 -O3 $(OPTION) -c -fPIC
LFLAGS = -std=c++14 -O3 $(OPTION)

OBJS = enum_rule.o Rules.o CSVReader.o Reader.o Table.o Aggregator.o Synthesize.o Wrapper.o Consolidation.o

all : _goldenrecord.so
	rm *.o

_goldenrecord.so: $(OBJS)
	  $(CC) -shared $^ -o $@

enum_rule : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $@

enum_rule.o : enum_rule.cc CSVReader.o Reader.o Table.o Rules.o 
	$(CC) $(CFLAGS) $< -o $@ 

Aggregator.o : Aggregator.cc Synthesize.o Wrapper.o
	$(CC) $(CFLAGS) $< -o $@ 

Synthesize.o : Synthesize.cc Synthesize.h Wrapper.o
	$(CC) $(CFLAGS) $< -o $@ 

Wrapper.o : Wrapper.cc 
	$(CC) $(CFLAGS) $< -o $@ 

Consolidation.o : Consolidation.cc Wrapper.o
	$(CC) $(CFLAGS) $< -o $@ 

Rules.o : Rules.cc 
	$(CC) $(CFLAGS) $< -o $@ 

CSVReader.o : CSVReader.cc Reader.o Table.o 
	$(CC) $(CFLAGS) $< -o $@ 

Reader.o : Reader.cc Table.o 
	$(CC) $(CFLAGS) $< -o $@ 

Table.o : Table.cc 
	$(CC) $(CFLAGS) $< -o $@ 

clean :
	rm enum_rule
	rm *.o
