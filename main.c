#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>

#define ARGS_MAX 1024
#define TOKEN_SIZE (ARGS_MAX / 2+1)

//method definitions
void redirIn(char*);
void redirOut(char*);

/**
 * parseInput splits the buffer into tokens. It accomplishes this 
 * by using certain delimiters that are useful and can be in the buffer
 * either by accident or on purpose by the user.  
 */
int parseInput(char* buffer, char* tokens[]){
    int i = 0;
    char* current_token = strtok(buffer, " \t\r\n");
    
    while(current_token != NULL){
        tokens[i] = current_token;
        i++;
        current_token = strtok(NULL,  " \t\r\n");
    }
    tokens[i] = NULL;
    return i;
}

/**
 * trim_input will take set up the buffer and 
 * 'parse' the command the user inputed (checking for run_background flag)
 */
void trim_input(char* buffer, int length, char* tokens[], _Bool* run_background){
    *run_background = false;

    //add NULL
    buffer[length] = '\0';
    if(buffer[strlen(buffer)-1] == '\n'){
        buffer[strlen(buffer)-1] = '\0';
    }
    int num_tokens = parseInput(buffer, tokens);
    if(num_tokens == 0){
        return;
    }
    if(num_tokens > 0 && strcmp(tokens[num_tokens - 1], "&") == 0){
        *run_background = true;
        tokens[num_tokens - 1] = 0;
    }
}

/**
 * Read a command into the buffer and tokenize it
 * tokens[i] will point to 'buffer' to the next iterated token in the command.
 */
void commandReader(char* buffer, char* tokens[], _Bool* run_background){
    int length = read(STDIN_FILENO, buffer, ARGS_MAX);
    if((length < 0)){
        perror("\nUnable to read command. Terminating...\n");
        exit(-1);
    }
    trim_input(buffer, length, tokens, run_background);
}


void executePipe(char* buf[], int nr, char* tokens[]){
    if(nr>10) return;
	
	int fd[10][2],i,pc;
	char *argv[100];

	for(i=0;i<nr;i++){
		parseInput(buf[i], tokens);
		if(i!=nr-1){
			if(pipe(fd[i])<0){
				perror("pipe creating was not successfull\n");
				return;
			}
		}
		if(fork()==0){//child1
			if(i!=nr-1){
				dup2(fd[i][1],1);
				close(fd[i][0]);
				close(fd[i][1]);
			}

			if(i!=0){
				dup2(fd[i-1][0],0);
				close(fd[i-1][1]);
				close(fd[i-1][0]);
			}
			execvp(argv[0],argv);
			perror("invalid input ");
			exit(1);//in case exec is not successfull, exit
		}
		//parent
		if(i!=0){//second process
			close(fd[i-1][0]);
			close(fd[i-1][1]);
		}
		wait(NULL);
	}
}


/**
 * Executes the users commands using fork in order to run them as child
 * processes. We check for the run_backgroundn flag.
 */
void commandExecution(char* tokens[], _Bool run_background){
    int fd,i = 0;
    _Bool in,out,pipeOn = false;
    char redir_input[512], redir_output[512], pipe_command[512];

    if(strcmp(tokens[0], "exit") == 0){
        printf("\n\nBye :(\n");
        exit(0);
    }else if(strcmp(tokens[0], "cd") == 0){
        if(chdir(tokens[1]) != 0){
            write(STDOUT_FILENO, "Invalid directory\n", strlen("Invalid directory\n"));
        }
        return;
    }

    //BEIGN FORK
    int status;
    pid_t p = fork();
    if(p == 0){
        //CHECK FOR <,>,|
        for(i=0; tokens[i] != NULL; i++){
            if(strcmp(tokens[i], ">")==0){
                tokens[i] = NULL;
                redirOut(strcpy(redir_output, tokens[i+1]));
            }else if(strcmp(tokens[i], "<")==0){
                tokens[i] = NULL;
                redirIn(strcpy(redir_input, tokens[i+1]));
            }else if(strcmp(tokens[i], "|")==0){
                tokens[i] = NULL;
                strcpy(pipe_command, tokens[i+1]);
                executePipe(tokens, 1, tokens);
            }
        }
        //child executes
        if(execvp(tokens[0], tokens) == -1){
            write(STDOUT_FILENO, tokens[0], strlen(tokens[0]));
            write(STDOUT_FILENO, " :command not found.\n", strlen(" :command not found.\n"));
            exit(-1);
        }
    }else if(!run_background){
		do {
			waitpid(p, &status, WUNTRACED);
            
		} while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }
}

/**
 * Gets the PS1 value of the user's machine
 * if the value is Null, then we set ('$ ') as their default.
 */
void getPS1(){
    char *ps1_user = getenv("PS1");
    if(ps1_user == NULL){
        write(STDOUT_FILENO, "$ ", strlen("$ "));
    }else{
        write(STDOUT_FILENO, ps1_user,  strlen(ps1_user));
        write(STDOUT_FILENO, "> ",  strlen("> "));
    }
}
/**
 * flush the buffer in order to prevent 
 * execution of the last command in the buffer. 
 */
void free_buffer(char *user_input){
    user_input[0] = '\n';
    int i;
    for(i=1; i<ARGS_MAX; i++){
        user_input[i] = '\0';
    }
}

int main(int argc, char* argv[]){
    char user_input[ARGS_MAX];
    char* user_token[TOKEN_SIZE];

    system("clear");
    printf("\n*******************Created by: Jonathan Argumedo and Julio Hernandez*******************\n");
    while(1){
        getPS1();
        
        _Bool run_background = false;
        commandReader(user_input, user_token, &run_background);
        if(strlen(user_input) == 0){
            continue;
        }

        commandExecution(user_token, run_background);
        free_buffer(user_input);
    }
    return 0;
}

//REDIRECTION AND PIPE METHODS
void redirOut(char* redir_output){
    int fd=0;
    if((fd = creat(redir_output, 0644)) < 0){
        perror(" :FILE COULD NOT BE OPEN\n");
        exit(0);
    }
    dup2(fd, STDOUT_FILENO);
    close(fd);
}

void redirIn(char* redir_input){
    int fd=0;
    if((fd = open(redir_input, O_RDONLY, 0644)) < 0){
        perror(" :FILE COULD NOT BE READ\n");
        exit(0);
    }
    dup2(fd, 0);
    close(fd);
}