#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

void handler (int sigNum) {
	char *name = strsignal(sigNum);
	write(STDOUT_FILENO, "\nLooper handling ",18);
	write(STDOUT_FILENO, name, strlen(name));
	write(STDOUT_FILENO, "\n", 2);
	signal(sigNum, SIG_DFL);
	raise(sigNum);

	if (sigNum == SIGTSTP) {
		signal(SIGCONT, handler);
	}
	else if (sigNum == SIGCONT) {
		signal(SIGTSTP, handler);
	}
}

int main(int argc, char **argv){ 

	printf("Starting the program\n");
	signal(SIGINT, handler);
	signal(SIGTSTP, handler);
	signal(SIGCONT, handler);


	while(1) {
		sleep(2);
	}
	return 0;
}