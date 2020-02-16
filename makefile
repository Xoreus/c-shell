all:
	gcc c-shell.c -o run
	./run
clean:
	rm run