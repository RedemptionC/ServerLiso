CC=gcc
CFLAGS=-I.
DEPS = parse.h y.tab.h
OBJ = y.tab.o lex.yy.o parse.o 
FLAGS = -g -Wall
LDFLAGS = -lpthread

default:all
 
all: lisod example 

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

run:
	./lisod 15441

test-big:
	python3 cp1_checker.py  127.0.0.1 15441 1000 10 500
	# python og_cp1_checker.py  127.0.0.1 15441 1000 10 500
test-small:
	# trails writeread connection
	python3 cp1_checker.py  127.0.0.1 15441 10 20 20
	# python og_cp1_checker.py  127.0.0.1 15441 10 20 20
test-remote:
	python3 cp1_checker.py   47.115.3.250 15441 10 20 20

clean:
	rm -f *~ *.o example lex.yy.c y.tab.c y.tab.h lisod
rebuild:
	make clean
	make 
