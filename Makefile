all: parse

ctrl: parse.cpp
	g++ -o parse parse.c

clean:
	rm -f parse