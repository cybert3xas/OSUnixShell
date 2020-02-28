# Unix Shell OS Lab #2
By: Jonathan Argumedo & Julio Hernandez

## Objective

The purpose of this lab is no simulate the behaviour of a real shell terminal in C.

### How to run

A Makefile has been created to facilitate the execution of the shell program. At this time there 
are three ways to achieve this.

```
1. On a terminal launch the shell by using the 'make' command.
    This approach will launch a seperate window and simulates a real shell.
    In addition, this approach assumes that you are using a linux machine.
    If this is not the case use method 2 or 3.

2. On a terminal launch the shell by using the 'make light' command.
    This method will complile and launch the C program on the current terminal.

3. On a terminal use the command "make shell".
    This is compile the code and you can launch the program by using the './shell'
    command.

In order to clean use the 'make clean' command.
```