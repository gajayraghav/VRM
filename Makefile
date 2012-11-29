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
TEST1=		test/test1.c
TEST2=		test/test2.c
TEST3=		test/test3.c
TEST4=		test/test4.c
TEST5=		test/test5.c
MULTIABORT=	test/multi-abort.c
TRUNCATE=	test/truncate.c
INC =		inc/
TARGET =	RVM


all:	$(LIB) $(BASIC)
basic:	$(INC) $(BASIC) $(LIB)
	$(CXXFLAGS) basic -I $(INC) $(BASIC) $(LIB)
abort:	$(INC) $(ABORT) $(LIB)
	$(CXXFLAGS) abort -I $(INC) $(ABORT) $(LIB)
multiabort:	$(INC) $(MULTIABORT) $(LIB)
	$(CXXFLAGS) multiabort -I $(INC) $(MULTIABORT) $(LIB)
truncate:	$(INC) $(TRUNCATE) $(LIB)
	$(CXXFLAGS) truncate -I $(INC) $(TRUNCATE) $(LIB)
multi:	$(INC) $(MULTI) $(LIB)
	$(CXXFLAGS) multi -I $(INC) $(MULTI) $(LIB)
test1:	$(INC) $(TEST1) $(LIB)
	$(CXXFLAGS) test1 -I $(INC) $(TEST1) $(LIB)
test2:	$(INC) $(TEST2) $(LIB)
	$(CXXFLAGS) test2 -I $(INC) $(TEST2) $(LIB)
test3:	$(INC) $(TEST3) $(LIB)
	$(CXXFLAGS) test3 -I $(INC) $(TEST3) $(LIB)
test4:	$(INC) $(TEST4) $(LIB)
	$(CXXFLAGS) test4 -I $(INC) $(TEST4) $(LIB)
test5:	$(INC) $(TEST5) $(LIB)
	$(CXXFLAGS) test5 -I $(INC) $(TEST5) $(LIB)
	

$(LIB):	$(OBJ)
	$(LIBFLAGS) $(LIB) $(OBJ)	
$(OBJ):	$(LIBS) $(SRC)
	$(COBJFLAGS) -I $(INC) $(SRC)
	mv *.o $(OBJ) 	
clean:
	rm -f $(OBJ) $(TARGET) $(LIB)