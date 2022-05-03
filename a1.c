#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        fprintf(stderr, "Usage: %s <string>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // create two pipes:
    // - parent_fds used by parent to write to child
    // - child_fds used by child to write to parent
    int parent_fds[2], child_fds[2];

    // read:[0] - write:[1]
    if (pipe(parent_fds) != 0 || pipe(child_fds) != 0)  /* || not && */
    {
        fprintf(stderr, "pipes failed!\n");
        return EXIT_FAILURE;
    }

    // fork() child process
    int child = fork();

    if (child < 0)
    {
        fprintf(stderr, "fork failed!");
        return EXIT_FAILURE;
    }
    else if (child == 0)
    {
        printf("%d: I reached the child :)\n", (int)getpid());
        // close unwanted pipe ends by child
        close(child_fds[0]);
        close(parent_fds[1]);

        // read from parent pipe
        char fromParent[100];
        int n = read(parent_fds[0], fromParent, sizeof(fromParent) - 1);
        fromParent[n] = '\0';
        printf("%d: Child: read from parent pipe '%s'\n", (int)getpid(), fromParent);
        close(parent_fds[0]);

        // Append to what was read in
        strcat(fromParent, " (added this.)");

        write(child_fds[1], fromParent, strlen(fromParent));
        close(child_fds[1]);
        printf("%d: Child: writing to pipe - '%s'\n", (int)getpid(), fromParent);
    }
    else
    {
        // close unwanted pipe ends by parent
        close(parent_fds[0]);
        close(child_fds[1]);

        // write from terminal to parent pipe FOR child to read
        printf("%d: Parent: writing to pipe '%s'\n", (int)getpid(), argv[1]);
        write(parent_fds[1], argv[1], strlen(argv[1]));
        close(parent_fds[1]);
        // read from child pipe
        char fromChild[100];
        int n = read(child_fds[0], fromChild, sizeof(fromChild) - 1);
        fromChild[n] = '\0';
        close(child_fds[0]);
        printf("%d: Parent: read from pipe - '%s'\n", (int)getpid(), fromChild);
    }

    int corpse;
    int status;
    while ((corpse = wait(&status)) > 0)
        printf("%d: child PID %d exited with status 0x%.4X\n", (int)getpid(), corpse, status);

    return EXIT_SUCCESS;
}