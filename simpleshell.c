#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#ifdef LINUX
typedef int uint32
#endif
#ifdef WINDOWS
typedef long uint32
#endif
// Below 2 lines are for the function readline()
//#define SPACE 0
//#define ALPHA 1

char *readline(void);
int main(int argc, char *argv[]) {
        char myprompt[] = "vaibshell:~$ ", *line = NULL;
	char *temp = NULL, *str, *token, *saveptr, *command, **argp;
	int i, j, status;
	pid_t pid, w;
	char malloc_error[] = "malloc failed:\n";
        while(1) {
                write(1, "\n", 1);
                write(1, myprompt, strlen(myprompt));
                //sleep(2);
                line = readline();
		argp = (char **) malloc((((strlen(line) + 1) / 2) + 1)* sizeof(char *));
		if(argp == NULL) {
			write(1, malloc_error, sizeof(malloc_error));
			exit(EXIT_FAILURE);
		}
                //write(1, line, strlen(line));
                //Now, check the no. of void spaces in the input excluding it at the first and at the end
		temp = (char *)malloc(strlen(line) + 1);
		if(temp == NULL) {
			write(1, malloc_error, sizeof(malloc_error));
			exit(EXIT_FAILURE);
		}
		strcpy(temp, line);
		i = 0;
		for(j = 1, str = temp;   ; j++, str = NULL) {
			token = strtok_r(str, " ", &saveptr);
			if(token == NULL) {
				break;
			}
			if(j == 1) {
				command = token;
				printf("command = %s\n", command);
			}
			else {
				argp[i++] = token;      //Don't we have to allocate memory for each argp[i] ? or it is allocated already?
				printf("argument[%d] =  %s\n", i - 1, argp[i - 1]);
			}
		}
		argp[i] = NULL;
		
		// Code to check whether strtok works or not? by printing the contents of command and arguments
		/*printf("%s ", command);
		i = 0;
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
			printf("Child pid is %ld \n", (long) getpid());
			execvp(command, argp);
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
					printf("exited, status=%d\n", WEXITSTATUS(status));
				}
				else if(WIFSIGNALED(status)) {
					printf("killed by signal %d\n", WTERMSIG(status));
				}
				else if(WIFSTOPPED(status)) {
					printf("stopped by signal %d\n", WSTOPSIG(status));
				}
				else if(WIFCONTINUED(status)) {
					printf("continued\n");
				}
			} while(!WIFEXITED(status) && !WIFSIGNALED(status));
		}
		
		free(line);
		free(temp);
		free(argp);
        }
        return 0;
}

char *readline() {
        int ch;//what will be better char or int?
        int i;
        char *str;
        int SIZE = 32;
        str = (char *) malloc(SIZE);
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
