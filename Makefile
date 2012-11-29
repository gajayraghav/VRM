CXXFLAGS =	g++ -g -o 
COBJFLAGS=	g++ -g -c
LIBFLAGS=       ar cr  
OBJPATH=	obj/
OBJ =		obj/RVM.o
SRC =		src/RVM.cpp
LIB =		obj/RVM.a	
BASIC=		test/basic.c
ABORT=		test/abort.c
MULTI=		test/multi.c
MULTIABORT=	test/multi-abort.c
TRUNCATE=	test/truncate.c
INC =		inc/
TARGET =	RVM


all:	$(LIB) $(BASIC)
basic:	$(INC) $(BASIC) $(LIB)
	$(CXXFLAGS) basic -I $(INC) $(BASIC) $(LIB)
abort:	$(INC) $(ABORT) $(LIB)
	$(CXXFLAGS) abort -I $(INC) $(ABORT) $(LIB)
multiabort:	$(INC) $(ABORT) $(LIB)
	$(CXXFLAGS) multiabort -I $(INC) $(MULTIABORT) $(LIB)
truncate:	$(INC) $(ABORT) $(LIB)
	$(CXXFLAGS) truncate -I $(INC) $(TRUNCATE) $(LIB)
multi:	$(INC) $(ABORT) $(LIB)
	$(CXXFLAGS) multi -I $(INC) $(MULTI) $(LIB)
$(LIB):	$(OBJ)
	$(LIBFLAGS) $(LIB) $(OBJ)	
$(OBJ):	$(LIBS) $(SRC)
	$(COBJFLAGS) -I $(INC) $(SRC)
	mv *.o $(OBJ) 	
clean:
	rm -f $(OBJ) $(TARGET) $(LIB)