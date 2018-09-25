all: mysh1 mysh2 mysh3 mysh4 syn_process_1 syn_process_2 syn_thread_1 syn_thread_2 Syn1

mysh1: mysh1.o
	gcc -o mysh1 mysh1.o

mysh2: mysh2.o
	gcc -o mysh2 mysh2.o

mysh3: mysh3.o
	gcc -o mysh3 mysh3.o

mysh4: mysh4.o
	gcc -o mysh4 mysh4.o

syn_process_1: syn_process_1.o
	gcc syn_process_1.c display.c -o syn_process_1

syn_process_2: syn_process_2.o
	gcc syn_process_2.c display.c -o syn_process_2

syn_thread_1: syn_thread_1.o
	gcc syn_thread_1.c display.c -o syn_thread_1 -lpthread

syn_thread_2: syn_thread_2.o
	gcc syn_thread_2.c display.c -o syn_thread_2 -lpthread

Syn1: Syn1.java
	javac Syn1.java Display.java


clean:
	rm -f mysh1 mysh2 mysh3 mysh4 syn_process_1 syn_process_2 syn_thread_1 syn_thread_2 mysh1.o mysh2.o mysh3.o mysh4.o syn_process_1.o syn_process_2.o syn_thread_1.o syn_thread_2.o Syn1.class Display.class
