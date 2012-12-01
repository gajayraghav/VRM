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
TEST20=		test/test20.c
SRI1=		test/sri_basicint.c
SRI2=		test/sri_multi.c
SRI3=		test/sri_multiseg.c
SRI4=		test/sri_new.c
ARCH1=		test/arch_basicint.c
ARCH2=		test/arch_twostring.c
OVERLAP=		test/overlap.c
MULTIABORT=	test/multi-abort.c
TRUNCATE=	test/truncate.c
INC =		inc/
TEST6=		test/test6.c
EXPAND=		test/expand.c
TARGET =	RVM


whole:	$(LIB) abort multi multiabort test1 test2 basic test3 test4 test5 arch1 arch2 sri1 sri2 overlap test6 expand

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
test6:	$(INC) $(TEST6) $(LIB)
	$(CXXFLAGS) test6 -I $(INC) $(TEST6) $(LIB)
test20:	$(INC) $(TEST20) $(LIB)
	$(CXXFLAGS) test20 -I $(INC) $(TEST20) $(LIB)


sri1:	$(INC) $(SRI1) $(LIB)
	$(CXXFLAGS) sri1 -I $(INC) $(SRI1) $(LIB)
sri2:	$(INC) $(SRI2) $(LIB)
	$(CXXFLAGS) sri2 -I $(INC) $(SRI2) $(LIB)
sri3:	$(INC) $(SRI3) $(LIB)
	$(CXXFLAGS) sri3 -I $(INC) $(SRI3) $(LIB)
sri4:	$(INC) $(SRI4) $(LIB)
	$(CXXFLAGS) sri4 -I $(INC) $(SRI4) $(LIB)

arch1:	$(INC) $(ARCH1) $(LIB)
	$(CXXFLAGS) arch1 -I $(INC) $(ARCH1) $(LIB)
arch2:	$(INC) $(ARCH2) $(LIB)
	$(CXXFLAGS) arch2 -I $(INC) $(ARCH2) $(LIB)

overlap:	$(INC) $(OVERLAP) $(LIB)
	$(CXXFLAGS) overlap -I $(INC) $(OVERLAP) $(LIB)
	
expand:	$(INC) $(EXPAND) $(LIB)
	$(CXXFLAGS) expand -I $(INC) $(EXPAND) $(LIB)	

$(LIB):	$(OBJ)
	$(LIBFLAGS) $(LIB) $(OBJ)	
$(OBJ):	$(LIBS) $(SRC)
	$(COBJFLAGS) -I $(INC) $(SRC)
	mv *.o $(OBJ) 	
clean:
	rm -f $(OBJ) $(TARGET) $(LIB)
	rm -f abort multi multiabort test1 test2 basic test3 test4 test5 arch1 arch2 sri1 sri2 overlap test6 expand
	rm -rf rvm_segments/
	clear