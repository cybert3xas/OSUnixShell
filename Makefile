all: 	
		make shell
		gnome-terminal -- /bin/sh -c './shell'

shell:  shell.o
		gcc shell.o -o shell

light: 
		make shell
		./shell

clean: 
		-rm *.o
		-rm shell