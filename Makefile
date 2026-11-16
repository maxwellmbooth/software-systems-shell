all: src/*.c src/*.h
	gcc -Wall -Wextra -g src/*.c -o bin/s3
