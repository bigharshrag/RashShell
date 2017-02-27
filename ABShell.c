#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
	while(1)
	{
		char inp[500];
		size_t argno;

		printf("ussh> ");
		while(fgets(inp, sizeof inp, stdin) != NULL)
		{
			inp[strlen(inp)-1] = '\0';
			const char s[2] = " ";
			char *token;
			argno = 1;
			char cmd[500];
			char **cargs = malloc(100 * sizeof(char*));
			int i , pid;

			token = strtok(inp, s);
			if(token != NULL){
				if(strcmp(token, "exit") == 0){
					exit(0);
				}
				strcpy(cmd, token);
				cargs[0] = malloc(32);
		   		strcpy(cargs[0], token);
			}
			else{
				printf("No input! Please enter a command\n");
				printf("ussh> ");
				continue;
			}

			token = strtok(NULL, s);
			while( token != NULL ) 
		   	{
			    cargs[argno] = malloc(32);
		   		strcpy(cargs[argno], token);
				token = strtok(NULL, s);
				argno++;
		   	}

		   	pid = fork();
		   	if(pid == 0){
		   		execvp(cmd, cargs);
		   	}
		   	else if(pid > 0){
		   		wait(0);
		   	}
		   	else{
		   		printf("Error executing the command. That's all I know.\n");
		   	}
			printf("ussh> ");
		}
	}
	return 0;
}