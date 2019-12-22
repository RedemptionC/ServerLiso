CC=gcc
CFLAGS=-I.
DEPS = parse.h y.tab.h
OBJ = y.tab.o lex.yy.o parse.o 
FLAGS = -g -Wall
LDFLAGS = -lpthread

default:all

all: lisod

lex.yy.c: lexer.l
	flex $^

y.tab.c: parser.y
	yacc -d $^

y.tab.h: y.tab.c

%.o: %.c $(DEPS)
	$(CC) $(FLAGS)  -c -o $@ $< $(CFLAGS)

example: $(OBJ) example.o
	$(CC) -o $@ $^ $(CFLAGS)

lisod: lisod.o csapp.o $(OBJ)   $(LDFLAGS) 
	$(CC) -o $@ $^ $(CFLAGS) $(FLAGS)
# hello:
# 	echo hello
clean:
	rm -f *~ *.o example lex.yy.c y.tab.c y.tab.h lisod
