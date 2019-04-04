CC = gcc
OUTPUT_DIR = ..

ls: ls.o record.o
	$(CC) ls.o record.o -o $(OUTPUT_DIR)/ls
	rm *.o

ls.o: ls.c
	$(CC) -c $<

record.o: record.c
	$(CC) -c $<

.PHONY: clean
clean:
	rm *.o
