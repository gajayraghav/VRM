CXXFLAGS =	-O2 -g -Wall -fmessage-length=0 

OBJS =		src/RVM.o

SRCS =		src/RVM.cpp

TESTS =		test/basic.c

LIBS =		inc/

TARGET =	RVM

$(TARGET):	$(OBJS)
	$(CXX) $(SRCS) $(TESTS) -I $(LIBS) -o $(TARGET)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
