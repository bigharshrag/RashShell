#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define TOKEN_BUFF_SIZE 16
#define TOKEN_DELIM " \t\r\n\a"

char **get_tokens(char * inp)
{
	int buff_size = TOKEN_BUFF_SIZE;
	char **cargs = malloc(buff_size * sizeof(char*)); // command and arguements
	char *token;
	int carg_no = 0;

	token = strtok(inp, TOKEN_DELIM);
	while(token != NULL)
	{
		cargs[carg_no] = token;
		++carg_no;

		if(carg_no > buff_size)
		{
			buff_size += TOKEN_BUFF_SIZE;
			cargs = realloc(cargs, buff_size * sizeof(char*)); // Increase the buffer
		}

		token = strtok(NULL, TOKEN_DELIM);;
	}

	cargs[carg_no] = NULL;
	return cargs;
}

void run_command(char** cargs)
{
	int pid;

	pid = fork();
	if(pid == 0){
		execvp(cargs[0], cargs);
	}
	else if(pid > 0){
		wait(0);
	}
	else{
		printf("Error executing the command. That's all I know.\n");
	}
}

int main()
{
	while(1)
	{
        char *inp = NULL;
        char **cargs;
        size_t len = 0; // for automatic allocation

		size_t argno;
        size_t b_read; //bytes read from getline()

		printf("ussh> ");
        while ((b_read = getline(&inp, &len, stdin)) != -1) 
        {
	        // printf("Retrieved inp of length %zu :\n", b_read);
            // printf("%s", inp);
			inp[b_read - 1] = '\0'; // remove last \n
			
			if(b_read == 1)
			{
				// Empty command
				continue;
			}

			cargs = get_tokens(inp);
			run_command(cargs);

			printf("ussh> ");
		}
	}
	return 0;
}