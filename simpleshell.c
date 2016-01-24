#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef LINUX
typedef int uint32
#endif
#ifdef WINDOWS
typedef long uint32
#endif
// Below 2 lines are for the function readline()
#define SPACE 0
#define ALPHA 1

char *readline(void);
int main() {
        char myprompt[] = "vaibshell:~$ ", *line = NULL;
        while(1) {
                write(1, "\n", 1);
                write(1, myprompt, strlen(myprompt));
                //sleep(2);
                line = readline();
                write(1, line, strlen(line));
                //Now, check the no. of void spaces in the input excluding it at the first and at the end
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
	state = SPACE;
        while(ch != '\n' && ch != EOF) { // After pressing Ctrl+D 2 times only then  it is detecting it, why?
		if(ch != ' ') {
			state = ALPHA;
		}
		if(state == ALPHA) {
			str[i++] = ch;
			if(i == SIZE - 1) {
				SIZE *= 2;
				str = realloc(str, SIZE);
				//if realloc fails, then what to do?
			}
		}
		ch = getchar();
        }
        // Here came means it exited from while either due to \n or EOF
        str[i] = '\0';
        return str;
}
