testlib: main.o list.o hash.o bitmap.o hex_dump.o
	gcc -o testlib main.o list.o hash.o bitmap.o hex_dump.o

main.o: main.c
	gcc -c main.c

list.o: list.c
	gcc -c list.c

hash.o: hash.c
	gcc -c hash.c

bitmap.o: bitmap.c
	gcc -c bitmap.c

hex_dump.o: hex_dump.c
	gcc -c hex_dump.c

clean:
	rm *.o
	rm testlib