CC := gcc
CFLAGS := -Wall -Wextra

my_pstree: my_pstree.c
	$(CC) $(CFLAGS) -o my_pstree my_pstree.c

.PHONY: clean
clean:
	rm -f my_pstree

run: my_pstree.c
	$(CC) $(CFLAGS) -o my_pstree my_pstree.c
	./my_pstree
