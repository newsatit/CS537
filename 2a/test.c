#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
int main(int argc, char* argv[]) {

	printf("In wish!\n");
	int rc = fork();
	// everything below this line is called twice!
	if (rc == 0) {
		printf("Child process\n");
		
		char* my_argv[10];
		my_argv[0] = "/bin/ps";
		my_argv[1] = "-f";
		my_argv[2] = NULL;

		int exec_rc = execv("/bin/ps", my_argv);
		printf("Done Calling exec %d\n", exec_rc);

		sleep(10);
		printf("Done Sleeping\n");
	} else {
		printf("Parent process\n");
	}

}
