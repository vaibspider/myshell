#include <stdio.h> 
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <error.h>
#include <signal.h>

//#include <readline/readline.h>
//#include <readline/history.h>

#define MAX 256
// Below 2 lines are for the function readline()
//#define SPACE 0
//#define ALPHA 1
/*int thr;
#define OPENSSL_THREAD_DEFINES
#include <openssl/opensslconf.h>
#if defined(OPENSSL_THREADS)
thr = 1;
#else
thr = 0;
#endif*/

char *readline(void);
int main(int argc, char *argv[]) {
	int argcount, dir, pipes, narg, copy_start, tmp_i, tmp_j, k, ret, sig, p_read, p_write, Size, background = 0;
	int **pipefd;
	char **tmp_ptr;
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
	char *temp = NULL, *str, *token, *saveptr, *command, ***argp, *op_filename, *ip_filename;
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
	//printf("Threads = %d\n", thr);
	
	size = MAX - 1;
        while(1) {
		background = 0;
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
		//printf("completed reading line \n");
		
		/*code to check if line is empty */
		/* Exceptions handling */
		if(strlen(line) == 0) {
			continue;
		}
		//exits when "exit" command is used
		if(strcmp(line, "exit") == 0) {
			break;
		}
		
		/* */
		/*sig = sigaction();
		if(sig == -1) {
			perror("sigaction failed");
			exit(EXIT_FAILURE);
		}*/
		
		//printf("skiping the exit\n");
		/* code for checking the no. of pipes("|") in the input command by the user */
		i = 0;
		pipes = 0;
		//printf("starting to count no of pipes\n");
		while(line[i] != '\0') {
			//printf("line[%d] = %c\n", i, line[i]);
			if(line[i] == '|') {
				pipes++;
				//printf("'|' detected: pipes = %d\n", pipes);
			}
			i++;
		}
		//printf("no of pipes detected = %d\n", pipes);
		
		/*code for allocating storage for the file descriptors */
		if(pipes > 0) {
			pipefd = (int **)malloc(pipes * sizeof(int*));
			for(i = 0; i < pipes; i++) {
				pipefd[i] = (int *)malloc(2 * sizeof(int));
			}
		}

		/* code to malloc that no. of argument array which are one greater than the no of pipes in input command */
		Size = ((strlen(line) + 1) / 2) + 1;
		narg = pipes + 1;
		argp = (char ***) malloc(narg * sizeof(char **));
		for(j = 0; j < narg; j++) {
			argp[j] = (char **) malloc((((strlen(line) + 1) / 2) + 1)* sizeof(char *));
			if(argp[j] == NULL) {
				write(1, malloc_error, sizeof(malloc_error));
				exit(EXIT_FAILURE);
			}
		}
		for(i = 0; i < narg; i++) {
			for(j = 0; j < Size; j++) {
				argp[i][j] = NULL;
			}
		}
		//printf("malloc succeded for %d pipes\n", pipes);

		/*argp[0] = (char **) malloc((((strlen(line) + 1) / 2) + 1)* sizeof(char *));
		if(argp[0] == NULL) {
			write(1, malloc_error, sizeof(malloc_error));
			exit(EXIT_FAILURE);
		}*/
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
			argp[0][i++] = token;
			/*if(j == 1) {
				//command = token;
				argp[0][i++] = token;
				//printf("argument[%d] = %s\n", i - 1, argp[0][i - 1]);
			}
			else {
				argp[0][i++] = token;      //Don't we have to allocate memory for each argp[0][i] ? or it is allocated already?
				//printf("argument[%d] =  %s\n", i - 1, argp[0][i - 1]);
			}*/
		}
		argp[0][i] = NULL;
		//printf("tokenizing done successfully\n");
		n = i;
		argcount = i;
		
		/* exit */
		if(argp[0][0] != NULL) {
			if(strcmp(argp[0][0], "exit") == 0) {
				break;
			}
		}
		
		/* Exceptions handling */
		if(argcount == 0) {
			continue;	//Possible error: continue can be used for while() loop? right?
		}
		/*code to execute "cd <directoryname>" command */
		if(strcmp(argp[0][0], "cd") == 0) {
			if(argcount == 1) {
				argp[0][1] = "/home/vaibhav";
			}
			else if(argcount != 2) {
				fprintf(stderr, "Usage: %s <directory_path>\n", argp[0][0]);
				exit(EXIT_FAILURE);
			}
			dir = chdir(argp[0][1]);
			if(dir == -1) {
				perror("chdir:");
				exit(EXIT_FAILURE);
			}
			continue;
		}
		//printf("Came after cd code\n");

		/* code to implement multiple pipes and all possible combinations of < , > and | */
		//printf("Now running the code for pipes\n");
		i = 0;
		j = 0;
		while(i < narg - 1) {	// "narg - 1" since we want to iterate only upto last argp.
			//printf("outer loop: %d\n", i);
			j = 0;
			copy_start = 0;
			k = 0;
			while(argp[i][j] != NULL) {
				//printf("inner loop: %d\n", j);
				if(copy_start == 0 && strcmp(argp[i][j], "|") == 0) {
					//printf("pipe found:\n");
					copy_start = 1;
					tmp_i = i;
					tmp_j = j;
					//tmp_ptr = &(argp[i][j]);
					j++;
					continue;
				}
				if(copy_start == 1) {
					//printf("copy_start = 1: so about to copy %s from argp[%d][%d] to argp[%d][%d]\n", argp[i][j], i, j, i + 1, k);
					//strcpy(argp[i + 1][k], argp[i][j]);
					argp[i + 1][k] = argp[i][j];
					k++;
					//printf("copied %s from argp[%d][%d] to argp[%d][%d]\n", argp[i][j], i, j, i + 1, k);
				}
				j++;
			}
			argp[tmp_i][tmp_j] = NULL;
			//*tmp_ptr = NULL;
			argp[i + 1][j] = NULL;
			i++;
		}
		//printf("Code for pipes ran successfully\n");
		/*code to check whether above pipe wala code works or not by printing the contents of different argp[i]'s */
		//printf("Now printing the contents of each argp array:\n");
		i = 0;
		j = 0;
		//printf("\n");
		while(i < narg) {
			j = 0;
			//printf("arg[%d] : ", i);
			while(argp[i][j] != NULL) {
				//printf("%s ", argp[i][j]);
				j++;
			}
			//printf("\n");
			i++;
		}
		//printf("printed the contents successfully & now continuing to the while loop again\n");
		//continue;	/* This continue; must be eliminated after I complete the code for pipe handling */
		
		/*code to detect ">" and take appropriate action*/
		/*i = 0;
		//printf("Came upto this!\n");
		while(argp[0][i] != NULL) {
			//printf("Came into while loop!\n");
			if(strcmp(argp[0][i++], ">") == 0) {
				strcpy(op_filename, argp[0][i]);
				argp[0][i - 1] = NULL;
				OUT_REDIR = 1;
				//printf("OUT_REDIR = %d\n", OUT_REDIR);
				break;
			}
		}*/
		/*code to detect "<" and take appropriate action*/
		/*i = 0;
		while(argp[0][i] != NULL) {
			if(strcmp(argp[0][i++], "<") == 0) {
				strcpy(ip_filename, argp[0][i]);
				argp[0][i - 1] = NULL;
				IN_REDIR = 1;
				//printf("IN_REDIR = %d\n", IN_REDIR);
				break;
			}
		}*/


		// Code to check whether strtok works or not? by printing the contents of command and arguments
		//printf("%s ", command);
		/*i = 0;
		while(1) {
			if(argp[0][i] != NULL)
				printf("%s ", argp[0][i++]);
			else 
				break;
		}
		printf("\n\n");*/
		//printf("Now starting the forking\n");
		//printf("narg = %d\n", narg);
		
		if(pipes > 0) {
			for(i = 0; i < pipes; i++) {
				ret = pipe(pipefd[i]);
				if(ret == -1) {
					perror("pipe failed:");	
					exit(EXIT_FAILURE);
				}
			}
		}
		p_read = 0;
		p_write = 0;
		for(i = 0; i < narg; i++, p_read++, p_write++) {
			if(i == 1) {
				p_read--;
			}
			//printf("p_read = %d  p_write = %d\n", p_read, p_write);
			pid = fork();
			//printf("fork %d done\n", i);
			if(pid == -1) {
				perror("fork");
				exit(EXIT_FAILURE);
			}
			if(pid == 0) {// that means it is child
				//printf("In child now:\n");

				/* code for Output and Input Redirection */
				/* code for identifying Input and Output Filenames */
				j = 0;
				while(argp[i][j] != NULL) {
					if(strcmp(argp[i][j], "&") == 0) {
						background = 1;
						printf("background = %d\n", background);
					}
					if(strcmp(argp[i][j], ">") == 0) {
						OUT_REDIR = 1;
						if(j == 0) {
							fprintf(stdout, "Usage: \"command < input > output\"\n or \"command > output < input\"\n"); 
						}
						strcpy(op_filename, argp[i][j + 1]);
					}
					else if(strcmp(argp[i][j], "<") == 0) {
						IN_REDIR = 1;
						if(j == 0) {
							fprintf(stdout, "Usage: \"command < input > output\"\n or \"command > output < input\"\n"); 
						}
						strcpy(ip_filename, argp[i][j + 1]);
					}
					j++;
				}
				
				/* code for putting NULL in the argp[i] in the place of first of "<" and ">"*/
				if(OUT_REDIR == 1 || IN_REDIR == 1) {
					//printf("out_redir = %d and in_redir = %d\n", OUT_REDIR, IN_REDIR);
					j = 0;
					while(argp[i][j] != NULL) {
						if(strcmp(argp[i][j], ">") == 0 || strcmp(argp[i][j], "<") == 0) {
							argp[i][j] = NULL;
							//tmp_i = i;
							//tmp_j = j;
							break;
						}
						j++;
					}
					//argp[tmp_i][tmp_j] = NULL;
				}
				if(background == 1) {
					printf("came in background\n");
					j = 0;
					while(argp[i][j] != NULL) {
						if(strcmp(argp[i][j], "&") == 0) {
							argp[i][j] = NULL;
							break;
						}
						j++;
					}
				}
				
				/* code to print the contents of argp[i] */
				k = 0;
				j = 0;
				/*printf("\n");
				while(k < narg) {
					j = 0;
					printf("arg[%d] : ", k);
					while(argp[k][j] != NULL) {
						printf("%s ", argp[k][j]);
						j++;
					}
					printf("\n");
					k++;
				}*/
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
				

				//printf("i = %d\n", i);
				if(narg > 1) {
					if(i == 0) {
						close(pipefd[p_write][0]);
						if(OUT_REDIR == 0) {
							STDOUT = dup(STDOUT_FILENO);
							if(STDOUT == -1) {
								perror("dup stdout failed");
								exit(EXIT_FAILURE);
							}
							//close(1);
							if(dup2(pipefd[p_write][1], STDOUT_FILENO) == -1) {
								write(STDOUT, "dup2 error\n", 12);
								exit(EXIT_FAILURE);
							}
							//write(STDOUT, "pipefd out succeded\n", 21);
						}
						
					}
					else if(i == narg - 1) {
						//printf("came in stdin change:\n");
						close(pipefd[p_read][1]);
						//printf("pipefd[0][1] done\n");
						/*if(dup2(0, STDIN) == -1) {
							printf("error pipefd in\n");
							//write(1, "dup2 error: 1\n", 14);
							perror("dup2 error:");
							exit(EXIT_FAILURE);
						}*/
						/*STDIN = dup(STDIN_FILENO);
						if(STDIN == -1) {
							perror("dup stdin failed");
							exit(EXIT_FAILURE);
						}*/
						//close(1);
						//close(0);
						//printf("no error pipefd IN :\n");
						if(IN_REDIR == 0) {
							if(dup2(pipefd[p_read][0], STDIN_FILENO) == -1) {
								//printf("error\n");
								//write(STDOUT, "dup2 error\n", 12);
								perror("dup2 IN error:");
								exit(EXIT_FAILURE);
							}
							//printf("came finally:\n");
							//write(1, "pipefd in succeded\n", 20);
						}
					}
					else {
						
						//printf("now in else pipefd:\n");
						close(pipefd[p_read][1]);
						close(pipefd[p_write][0]);
						if(IN_REDIR == 0) {
							if(dup2(pipefd[p_read][0], STDIN_FILENO) == -1) {
								perror("pipefd IN failed:\n");
								exit(EXIT_FAILURE);
							}
							//printf("3 complete\n");
						}
						if(OUT_REDIR == 0) {
							STDOUT = dup(STDOUT_FILENO);
							if(STDOUT == -1) {
								perror("dup stdout failed");
								exit(EXIT_FAILURE);
							}
							if(dup2(pipefd[p_write][1], STDOUT_FILENO) == -1) {
								write(STDOUT, "pipefd else failed:\n", 20);
								//perror("pipefd OUT failed:\n");
								exit(EXIT_FAILURE);
							}
							//write(STDOUT, "pipefd else succeded\n", 21);
						}
					}
				}
				//printf("now doing execvp\n");
				//printf("Child pid is %ld \n", (long) getpid());
				exec = execvp(*argp[i], argp[i]);
				if(exec == -1) {
					perror("exec failed");
					exit(EXIT_FAILURE);
				}
			}
			else {		//that means it is parent
				// I didn't understand why we need to use loop, why can't we use waitpid() just once?
				if(background == 0) {
					if(pipes == 0) {
						do {
							//printf("parent waiting...\n");
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
					if(pipes > 0) {
						if(i == 0) {
							close(pipefd[p_write][1]);
						}
						else if(i == narg - 1) {
							close(pipefd[p_read][0]);
						}
						else {
							close(pipefd[p_read][0]);
							close(pipefd[p_write][1]);
						}
					}
				}
			}
		}
		
		free(line);
		free(temp);
		for(j = 0; j < narg; j++) {
			free(argp[j]);
		}
		free(argp);
		if(pipes > 0) {
			for(i = 0; i < pipes; i++) {
				free(pipefd[i]);
			}
			free(pipefd);
		}
		close(fd_in);
		close(fd_out);
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
