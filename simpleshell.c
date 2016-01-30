#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <error.h>

#define MAX 256
// Below 2 lines are for the function readline()
//#define SPACE 0
//#define ALPHA 1

char *readline(void);
int main(int argc, char *argv[]) {
	int argcount, dir;
	char malloc_error[] = "malloc failed:\n";
	char *cwd, *buf = (char *) malloc(MAX * sizeof(char));
	int size;
	if(buf == NULL) {
		write(1, malloc_error, sizeof(malloc_error));
		exit(EXIT_FAILURE);
	}
        char *myprompt = (char *) malloc(MAX * sizeof(char));;
	if(myprompt == NULL) {
		write(1, malloc_error, sizeof(malloc_error));
		exit(EXIT_FAILURE);
	}
        char *line = NULL;
	char *temp = NULL, *str, *token, *saveptr, *command, **argp, *op_filename, *ip_filename;
	int i, j, status, exec = 0, n;
	pid_t pid, w;
	int fd_out, OUT_REDIR, STDOUT; 
	int fd_in, IN_REDIR, STDIN; 
	op_filename = (char *) malloc(MAX * sizeof(char));
	if(op_filename == NULL) {
		write(1, malloc_error, sizeof(malloc_error));
		exit(EXIT_FAILURE);
	}
	ip_filename = (char *) malloc(MAX * sizeof(char));
	if(ip_filename == NULL) {
		write(1, malloc_error, sizeof(malloc_error));
		exit(EXIT_FAILURE);
	}
	
	size = MAX - 1;
        while(1) {
		OUT_REDIR = 0;
		IN_REDIR = 0;


                write(1, "\n", 1);
		/*code to specify what to show as a prompt depending on the current directory */
		do {
			cwd = getcwd(buf, size);
			if(cwd == NULL) {
				size *= 2;
			}
		}while(cwd == NULL);
		sprintf(myprompt, "vaibshell:%s$ ", cwd);
		//strcpy(myprompt, "vaibshell:~$ ");
                write(1, myprompt, strlen(myprompt));


                //sleep(2);
                line = readline();
		
		/*code to check if line is empty */
		/* Exceptions handling */
		if(strlen(line) == 0) {
			continue;
		}
		//exits when "exit" command is used
		if(strcmp(line, "exit") == 0) {
			break;
		}
		
		argp = (char **) malloc((((strlen(line) + 1) / 2) + 1)* sizeof(char *));
		if(argp == NULL) {
			write(1, malloc_error, sizeof(malloc_error));
			exit(EXIT_FAILURE);
		}
		//write(1, line, strlen(line));
		temp = (char *)malloc((strlen(line) + 1) * sizeof(char));
		if(temp == NULL) {
			write(1, malloc_error, sizeof(malloc_error));
			exit(EXIT_FAILURE);
		}
		/* temp used since strtok changes the argument string */
		strcpy(temp, line);
		i = 0;
		/* Code to tokenize the input string */
		/* I didn't understand how after executing command "ls > filename", the fd "1" is reset to the previous stdout file? */
		for(str = temp;   ; str = NULL) {
			token = strtok_r(str, " ", &saveptr);		//What is meant by threadsafe?  what is there in saveptr?
			if(token == NULL) {
				break;
			}
			argp[i++] = token;
			/*if(j == 1) {
				//command = token;
				argp[i++] = token;
				//printf("argument[%d] = %s\n", i - 1, argp[i - 1]);
			}
			else {
				argp[i++] = token;      //Don't we have to allocate memory for each argp[i] ? or it is allocated already?
				//printf("argument[%d] =  %s\n", i - 1, argp[i - 1]);
			}*/
		}
		argp[i] = NULL;
		n = i;
		argcount = i;
		
		/* Exceptions handling */
		if(argcount == 0) {
			continue;	//Possible error: continue can be used for while() loop? right?
		}
		/*code to execute "cd <directoryname>" command */
		if(strcmp(argp[0], "cd") == 0) {
			if(argcount == 1) {
				argp[1] = "/home/vaibhav";
			}
			else if(argcount != 2) {
				fprintf(stderr, "Usage: %s <directory_path>\n", argp[0]);
				exit(EXIT_FAILURE);
			}
			dir = chdir(argp[1]);
			if(dir == -1) {
				perror("chdir:");
				exit(EXIT_FAILURE);
			}
			continue;
		}
		
		/*code to detect ">" and take appropriate action*/
		i = 0;
		//printf("Came upto this!\n");
		while(argp[i] != NULL) {
			//printf("Came into while loop!\n");
			if(strcmp(argp[i++], ">") == 0) {
				strcpy(op_filename, argp[i]);
				argp[i - 1] = NULL;
				OUT_REDIR = 1;
				//printf("OUT_REDIR = %d\n", OUT_REDIR);
				break;
			}
		}
		
		/*code to detect "<" and take appropriate action*/
		i = 0;
		while(argp[i] != NULL) {
			if(strcmp(argp[i++], "<") == 0) {
				strcpy(ip_filename, argp[i]);
				argp[i - 1] = NULL;
				IN_REDIR = 1;
				//printf("IN_REDIR = %d\n", IN_REDIR);
				break;
			}
		}


		// Code to check whether strtok works or not? by printing the contents of command and arguments
		//printf("%s ", command);
		/*i = 0;
		while(1) {
			if(argp[i] != NULL)
				printf("%s ", argp[i++]);
			else 
				break;
		}
		printf("\n\n");*/
		
		pid = fork();
		if(pid == -1) {
			perror("fork");
			exit(EXIT_FAILURE);
		}
		if(pid == 0) {// that means it is child
			
			/* Code for output redirection */
			if(OUT_REDIR) {
				STDOUT = dup(1);
				close(1);
				fd_out = open(op_filename, O_WRONLY | O_CREAT, S_IRWXU);
				if(fd_out == -1) {
					perror("File open failed");
					exit(EXIT_FAILURE);
				}
				//printf("fd_out = %d\n", fd_out);
				
			}

			/* Code for input redirection */
			if(IN_REDIR) {
				STDIN = dup(0);
				close(0);
				fd_in = open(ip_filename, O_RDONLY);
				if(fd_in == -1) {
					perror("File open failed");
					exit(EXIT_FAILURE);
				}
				//printf("fd_in = %d\n", fd_in);
				
			}
			
			/* Code for pipe() */

			/*if(PIPE) {		// Assuming that only pipe is present. Mix of > , < and | will be done later.
				if(pipe(pipefd) == -1) {
					perror("pipe failed");
					exit(EXIT_FAILURE);
				}
				
			}*/

			
			//printf("Child pid is %ld \n", (long) getpid());
			exec = execvp(*argp, argp);
			if(exec == -1) {
				perror("exec failed");
				exit(EXIT_FAILURE);
			}
		}
		else {		//that means it is parent
			// I didn't understand why we need to use loop, why can't we use waitpid() just once?
			do {
				w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
				if(w == -1) {
					perror("waitpid");
					exit(EXIT_FAILURE);
				}
				if(WIFEXITED(status)) {
					//printf("exited, status=%d\n", WEXITSTATUS(status));
				}
				else if(WIFSIGNALED(status)) {
					//printf("killed by signal %d\n", WTERMSIG(status));
				}
				else if(WIFSTOPPED(status)) {
					//printf("stopped by signal %d\n", WSTOPSIG(status));
				}
				else if(WIFCONTINUED(status)) {
					//printf("continued\n");
				}
			} while(!WIFEXITED(status) && !WIFSIGNALED(status));
		}
		
		free(line);
		free(temp);
		free(argp);
        }
	free(ip_filename);
	free(op_filename);
	free(myprompt);
	free(buf);
        return 0;
}

char *readline() {
        int ch;//what will be better char or int?
        int i;
        char *str;
        int SIZE = 32;
        str = (char *) malloc(SIZE * sizeof(char));
        if(str == NULL) {
                char error[] = "Malloc failed\n";
                write(1, error, strlen(error));
                exit(1);// Is there a better mechanism to do this?
        }
        ch = getchar();
        i = 0;
	//state = SPACE;
        while(ch != '\n' && ch != EOF) { // After pressing Ctrl+D 2 times only then  it is detecting it, why?
		/*if(ch != ' ') {
			state = ALPHA;
		}*/
		//if(state == ALPHA) {
		str[i++] = ch;
		if(i == SIZE - 1) {
			SIZE *= 2;
			str = realloc(str, SIZE);
			//if realloc fails, then what to do?
		}
		//}
		ch = getchar();
        }
        // Here came means it exited from while either due to \n or EOF
        str[i] = '\0';
        return str;
}
