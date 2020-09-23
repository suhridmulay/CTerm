#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

#define EXIT_CODE "quit"
#define PATH_SIZE 256

#define ARGC_MAX 15

#define PROMPT ">>>"

char curr_path[PATH_SIZE];

struct statement {
    // A single commands
    char ** command;
    // Or two statements joined with an operator
    // Operator codes: 0 for serial
    //                 1 for parallel
    int operator;
    struct statement * operands[2];
};

void execute_stmt(struct statement);

void execute_wait(char ** command);
void execute_parallel(char ** command);

int main() {
    // Initialisation

    // -- Set line size
    size_t line_size = 1024;
    // -- Current working directory
    getcwd(curr_path, PATH_SIZE);
    // -- Running stores the terminal state
    int running = 1;
    // -- Allocate a linebuffer
    char *linebuf = (char *)malloc(sizeof(char) * line_size);
    if (linebuf == NULL) {
        fprintf(stderr, "ERROR: Unable to allocate line buffer, exiting");
        running = 0;
    }

    // Terminal loop
    while(running) {
        // Display terminal prompt
        printf("%s\n", curr_path);
        printf("%s", PROMPT);
        // Scan in the line
        getline(&linebuf, &line_size, stdin);
        // Remove trailing newline
        linebuf[strlen(linebuf) - 1] = '\0';
        // Exit code check
        int x = strcmp(linebuf, EXIT_CODE);
        if (strcmp(linebuf, EXIT_CODE) == 0) {
            running = 0;
        } else {
            // Run the input from the linebuffer
            printf("Tokenising input...\n");
            // Break it into tokens
            char * token = strtok(linebuf, " ");
            char ** cmd = malloc(sizeof(char *) * ARGC_MAX);
            printf("Executable: %s\n", token);
            int i = 0;
            cmd[i] = token;
            i++;
            while(token != NULL) {
                token = strtok(NULL, " ");
                printf("Argument: %s\n", token);
                cmd[i] = token;
                i++;
            }
            printf("Attempting execution\n");
            execute_wait(cmd);
        }
    }
}

void execute_wait(char ** command) {
    int pid = fork();
    if (pid == 0) {
        // Inside child
        // -- Start running said command
        int res = execvp(command[0], &command[0]);
        if (res == -1) {
            printf("Error loading executable: %s\n", command[0]);
        }
        printf("Process loading result: %d\n", res);
    } else if (pid > 0) {
        // Inside parent
        // -- Wait for the child
        int status;
        wait(&status);
        printf("Wait status: %d\n", status);
    }
}

void execute_parallel(char ** command) {
    int pid = fork();
    if (pid == 0) {
        // Inside child
        // -- Start running said command
        int res = execvp(command[0], &command[0]);
    } else if (pid > 0) {
        // Inside parent
        // -- Return control to shell
        return;
    }
}

void execute_stmt(struct statement s) {
    if (s.command == NULL) {
        if (s.operator == 0) {
            execute_stmt(*s.operands[0]);
            execute_stmt(*s.operands[1]);
        }
    }
}