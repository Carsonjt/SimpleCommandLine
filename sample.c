#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <pwd.h>

struct bg_pro {
	pid_t pid;
	char* command;
	int ac;
	struct bg_pro* next;
};
	int bg_count = 0;
	struct bg_pro* root;

void addBG(pid_t new_pid, char* new_command) {
	struct bg_pro *new_node = malloc(sizeof(struct bg_pro));
  
    new_node->pid  = new_pid;
	new_node->command = new_command;
	new_node->next = root;
	root = new_node;
}


void checkTer() {
	if(bg_count > 0) {
		pid_t ter = waitpid(0,NULL,WNOHANG);
		while(ter > 0) {
			struct bg_pro *cur = malloc(sizeof(struct bg_pro));
			cur = root;
			if(root->pid == ter) {
				printf("PID %d has been terminated.\n", root->pid);
				bg_count--;
				root = root->next;
			} else {
				while(cur->next->pid != ter) {
					cur = cur->next;
				}
				printf("PID %d has been terminated.\n", cur->next->pid);
				bg_count--;
				cur->next = cur->next->next;
				
			}
			ter = waitpid(0,NULL,WNOHANG);
		}
	}
}

 
void printBGs() {
	struct bg_pro* tempPointer = root;
	int i = 0;
	while(tempPointer != NULL) {
  		printf("ID:%d > Process number: %d\n", tempPointer->pid, i);
		i++;
  		tempPointer = tempPointer->next;
	}
}

int
main(int argc, char* argv[])
{

	char host_name[1024];
	char *user_name;
	user_name = getlogin();
	gethostname(host_name, 1024);
	char *user_host = strcat(   strcat(user_name, "@"),  strcat(host_name, ":")    );

	int bailout = 0;
	while (!bailout) {
		char cwd[256];
		getcwd(cwd, sizeof(cwd));

		char *cur;
		strcpy(cur,user_host);
		strcat(cur, cwd);
		strcat(cur, " > ");

		char* prompt = cur;
		char* reply = readline(prompt);
		char **args = NULL;
		char *p = strtok(reply, " ");
		int argc = 0, i;

		while(p) {
			args = realloc(args, sizeof(char*) * ++argc);
			if(args==NULL)
				exit(-1);

			args[argc-1] = p;
			p = strtok(NULL," ");
		}
		args = realloc(args, sizeof(char*) * argc+1);
		args[argc] = 0;


		//for (i = 0; i < (argc+1); ++i)
  		//	printf ("args[%d] = %s\n", i, args[i]);
		/* Note that readline strips away the final \n */
		/* For Perl junkies, readline automatically chomps the line read */

		if (!strcmp(reply, "bye")) {
			bailout = 1;

		} else if (!strcmp(reply, "cd")) {
			if (args[1] != NULL)
				if(strcmp(args[1], "~") == 0) {
					const char *homedir = getpwuid(getuid())->pw_dir;
					chdir(homedir);
				} else
					chdir(args[1]);
			else
				printf("%s", "Missing argument.\n");
		
		} else if (!strcmp(reply, "pwd")) {
			if (getcwd(cwd, sizeof(cwd)) != NULL)
       			printf("%s\n", cwd);

		} else if (!strcmp(reply, "bg")) {
			pid_t p = fork();
			if(p == 0) { // CHILD
				printf("%s\n", "");
				execvp(args[1],&args[1]);
				printf("%s\n", "Invalid command.");
			} else { // PARENT
				waitpid(p, NULL, 1);
				bg_count++;
				addBG(p, reply);
			}

		} else if (!strcmp(reply, "bglist")) {
			checkTer();
			printBGs();
			printf("Total background jobs: %d\n", bg_count);
		} else {
			if(fork() == 0) {
				execvp(args[0],args);
				printf("%s\n", "Invalid command.");
			}else
				waitpid(p, NULL, 0);
			
		}
	
		checkTer();

		free(args);
		free(reply);
	}
	printf("Exiting\n");
}
