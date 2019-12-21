CC=gcc
CFLAGS=-I.
DEPS = parse.h y.tab.h
OBJ = y.tab.o lex.yy.o parse.o example.o
FLAGS = -g -Wall
LDFLAGS = -lpthread

default:all

all: example lisod

lex.yy.c: lexer.l
	flex $^

y.tab.c: parser.y
	yacc -d $^

%.o: %.c $(DEPS)
	$(CC) $(FLAGS) -c -o $@ $< $(CFLAGS)

example: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

lisod: lisod.o csapp.o $(LDFLAGS)
	$(CC) -o $@ $^ $(CFLAGS) 

clean:
	rm -f *~ *.o example lex.yy.c y.tab.c y.tab.h lisod