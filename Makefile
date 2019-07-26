OBJ=parser.o lexanal.o main.o utils.o id.o code.o assemble.o compat.o
OBJ2=lib6502.o compat.o

CC=gcc
CFLAGS=-c
DEBUGFLAGS=-g

.y.c:
	bison -vd $*.y
	mv $*.tab.c $*.c
	mv $*.tab.h $*.h

.l.c:
	flex $*.l
	mv lex.yy.c $*.c

.c.o:
	$(CC) $(CFLAGS) $(DEBUGFLAGS) $*.c 

target:		as6502 lib6502
	
as6502:		$(OBJ)
		gcc -o $@ $(DEBUGFLAGS) $(OBJ)
		

lib6502:	$(OBJ2)
		gcc -o $@ $(DEBUFGLAGS) $(OBJ2)

clean:
		@rm $(OBJ) $(OBJ2) parser.h parser.output as6502 lib6502
