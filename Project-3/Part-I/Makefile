virtmem:
	gcc -o virtmem.o virtmem.c
run: 
	gcc -o virtmem.o virtmem.c && ./virtmem.o ./BACKING_STORE.bin addresses.txt
lru: 
	gcc -o virtmem.o virtmem.c && ./virtmem.o ./BACKING_STORE.bin addresses.txt -p 1
