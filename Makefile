CC = gcc

ls: ls.o record.o
	$(CC) ls.o record.o -o ls
	rm *.o

ls.o: ls.c
	$(CC) -c $<

record.o: record.c
	$(CC) -c $<

.PHONY: clean
clean:
	rm *.o