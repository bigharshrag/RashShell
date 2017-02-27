#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define CMD_BUFF_SIZE 16
#define TOKEN_DELIM " \t\r\n\a"

char **get_tokens(char * inp)
{
	int buff_size = CMD_BUFF_SIZE;
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
			buff_size += CMD_BUFF_SIZE;
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

void add_to_history(char *inp)
{
	FILE *history_fp;
	history_fp = fopen("rash_history", "a+");
	int ret = fputs(inp, history_fp);
	fputc('\n', history_fp);
	if(ret == EOF)
	{
		perror("rash: error in creating history");
	}
	fclose(history_fp);
}

void history_cmd()
{
	int c;
	FILE *history_fp;
	history_fp = fopen("rash_history", "r");
	if (history_fp) {
	    while ((c = getc(history_fp)) != EOF)
	        putchar(c);
	    fclose(history_fp);
	}

}

int custom_cmds(char** cargs)
{	
	int ret = 0;
	// check for known commands
	if(strcmp(cargs[0], "exit") == 0)
	{
		exit_cmd();
		ret = 1;
	}
	else if(strcmp(cargs[0], "cd") == 0)
	{
		cd_cmd(cargs[1]);
		ret = 1;
	}
	else if(strcmp(cargs[0], "history") == 0)
	{
		history_cmd();
		ret = 1;
	}
	return ret;
}

int handle_redir(char **cargs)
{
	// char **cmd1, **cmd2, **cmd3; 
	// cmd1 < cmd2 > cmd3
	int buff_size = CMD_BUFF_SIZE; // max can be allocated buffer
	char **cmd1 = malloc(buff_size * sizeof(char*));
	char *cmd2 = malloc(sizeof(char*));
	char *cmd3 = malloc(sizeof(char*));
	int i = 0, cmd_cnt = 0, pres_flag = 0;
	int in, out;

	int pid, status, w;
	int ret;

	while(cargs[i] != NULL)
	{
		if(strcmp(cargs[i], "<") == 0 || strcmp(cargs[i], ">") == 0)
		{
			pres_flag = 1;
		}
		++i;
	}

	if(pres_flag == 0)
	{
		return 0;
	}

	i = 0;

	while(!(strcmp(cargs[i], "<") == 0 || strcmp(cargs[i], ">") == 0))
	{
		// strcpy(cmd1[i], cargs[i]);
		cmd1[cmd_cnt] = cargs[i];
		++i; ++cmd_cnt;

		if(cmd_cnt > buff_size)
		{
			buff_size += CMD_BUFF_SIZE;
			cmd1 = realloc(cmd1, buff_size * sizeof(char*)); // Increase the buffer
		}
	}
	cmd1[cmd_cnt] = NULL;
	
	pid = fork();
	if(pid == 0)
	{
		if(strcmp(cargs[i], "<") == 0)
		{
			++i;
			cmd2 = cargs[i];
			// strcpy(cmd2, cargs[i]);
			in = open(cmd2, O_RDONLY);
			dup2(in, STDIN_FILENO);
			close(in);

			ret = execvp(cmd1[0], cmd1);
			if(ret == -1)
			{
				perror("rash_redir: command not found");
				exit(EXIT_FAILURE); // Use over exit(1) as in man page
			}
		}
		else if(strcmp(cargs[i], ">") == 0)
		{
			++i;
			cmd2 = cargs[i];
			
			out = open(cmd2, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
			dup2(out, STDOUT_FILENO);
			close(out);

			ret = execvp(cmd1[0], cmd1);
			if(ret == -1)
			{
				perror("rash_redir: command not found");
				exit(EXIT_FAILURE); // Use over exit(1) as in man page
			}
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

	free(cmd1);
	free(cmd2);
	free(cmd3);
	
	return 1;
}

void run_command(char** cargs)
{
	int pid, status, w;
	int ret;


	if(custom_cmds(cargs) == 1)
		return;
	else if(handle_redir(cargs) == 1)
		return;

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

			add_to_history(inp);

			cargs = get_tokens(inp);
			run_command(cargs);
			printf("rash> ");
		}
	}

	return 0;
}