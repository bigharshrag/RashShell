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

void exit_cmd()
{
	exit(EXIT_SUCCESS);
}

void cd_cmd(char *path)
{
	int ret = chdir(path);
	if(ret == -1)
	{
		perror("rash: cd command failed");
	}
}

void run_command(char** cargs)
{
	int pid, status, w;
	int ret;


	// check for known commands
	if(strcmp(cargs[0], "exit") == 0)
	{
		exit_cmd();
	}
	else if(strcmp(cargs[0], "cd") == 0)
	{
		cd_cmd(cargs[1]);
	}
	else{
	pid = fork();
	if(pid == 0)
	{
		ret = execvp(cargs[0], cargs);
		if(ret == -1)
		{
			perror("rash: command not found");
			exit(EXIT_FAILURE); // Use over exit(1) as in man page
		}
	}
	else if(pid > 0){
		do {
            w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
            if (w == -1) {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	else{
		printf("Error executing the command. That's all I know.\n");
	}
	}
}

int main()
{

	signal(SIGINT, SIG_IGN); // SIG_IGN ignores the signal

	while(1)
	{
        char *inp = NULL;
        char **cargs;
        size_t len = 0; // for automatic allocation

		size_t argno;
        size_t b_read; //bytes read from getline()

		printf("rash> ");
        while ((b_read = getline(&inp, &len, stdin)) != -1) 
        {
	        // printf("Retrieved inp of length %zu :\n", b_read);
            // printf("%s", inp);
			inp[b_read - 1] = '\0'; // remove last \n
			
			if(b_read == 1)
			{
				// Empty command
				printf("rash> ");
				continue;
			}
			cargs = get_tokens(inp);
			run_command(cargs);
			printf("rash> ");
		}
	}

	return 0;
}