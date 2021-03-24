CC=g++
CFLAGS=-c -Wall -Werror -pthread
all: zip
zip: thread_zip.o
thread_zip.o: thread_zip.cpp
	$(CC) thread_zip.cpp -o thread_zip $(CFLAGS)
clean: rm -f *o thread_zip
