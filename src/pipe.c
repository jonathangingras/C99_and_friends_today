#include <unistd.h>
#include <sys/fcntl.h>
#include <signal.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int parent(int in) {
	char buffer[257];

	FILE* istream = fdopen(in, "r");

	begin_parent:

		memset(buffer, 0, 257);
		fgets(buffer, 256, istream);

		printf("parent: %s\n", buffer);

	goto begin_parent;
}

int child(int out) {
	char buffer[257];

	FILE* ostream = fdopen(out, "w");

	begin_child:

		memset(buffer, 0, 257);
		fgets(buffer, 256, stdin);

		fprintf(ostream, "%s", buffer);

	goto begin_child;
}

int main(int argc, char **argv) {
	int pipe_[2];

	if(pipe(pipe_)) {
		perror("pipe");
		exit(1);
	}

	if(fork()) {
    close(pipe_[1]);
		return parent(pipe_[0]);
	} else {
    close(pipe_[0]);
		return child(pipe_[1]);
	}
}
