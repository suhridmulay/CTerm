build: src/terminal.c
	gcc src/terminal.c -o build/terminal

run: build/terminal
	./build/terminal

clean: build/terminal
	rm build/terminal