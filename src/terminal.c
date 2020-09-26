#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
#include<signal.h>

#define EXIT_CODE "exit"
#define PATH_SIZE 256

#define ARGC_MAX 15
#define CMD_MAX 32

#define PROMPT "$"

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

// Multitok taken from stack overflow
char *multi_tok(char *input, char *delimiter) {
    static char *string;
    if (input != NULL)
        string = input;

    if (string == NULL)
        return string;

    char *end = strstr(string, delimiter);
    if (end == NULL) {
        char *temp = string;
        string = NULL;
        return temp;
    }

    char *temp = string;

    *end = '\0';
    string = end + strlen(delimiter);
    return temp;
}

void execute_stmt(struct statement);

void execute_wait(char ** command);
void execute_parallel(char ** command);

void execute_multiple_serial(char *** commands);
void execute_multiple_parallel(char *** commands);

void write_output(char ** buffer, char * filename) {
    int pid = fork();
    if (pid == 0) {
        int fdr = open(filename, O_CREAT|O_WRONLY|O_TRUNC, 0777);
        int save_out = dup(fileno(stdout));
        dup2(fdr, fileno(stdout));
        signal(SIGTSTP, SIG_DFL);
        signal(SIGINT, SIG_DFL);
        execvp(buffer[0], &buffer[0]);
        dup2(save_out, fileno(stdout));
        fsync(fdr);
        close(fdr);
        exit(0);
    } else if (pid > 0) {
        return;
    }
}

void fancy_exit() {
    printf("use quit to exit terminal\n");
}

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
    if (*linebuf == EOF) {
        fprintf(stderr, "Messed up");
        running = 0;
    }
    signal(SIGTSTP, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    // Terminal loop
    while(running) {
        // Handle signals
        // Display terminal prompt
        printf("%s%s", curr_path, PROMPT);
        // Scan in the line
        getline(&linebuf, &line_size, stdin);
        // Remove trailing newline
        linebuf[strlen(linebuf) - 1] = '\0';
        // Exit code check
        int x = strcmp(linebuf, EXIT_CODE);
        if (strcmp(linebuf, EXIT_CODE) == 0) {
            printf("Exiting shell...\n");
            running = 0;
        } else {
            char * token = strtok(linebuf, " \n\t");
            char ** token_list = malloc(sizeof(char *) * ARGC_MAX);
            char ** cmd_buf = malloc(sizeof(char *) * CMD_MAX);
            int i = 0;
            int buf_start = 0;
            while(token != NULL) {
                if (linebuf == NULL) {
                    exit(-1);
                }
                // If token is not ## or &&
                int hh = strcmp(token, "##");
                int aa = strcmp(token, "&&");
                int redir = strcmp(token, ">");
                int cd = strcmp(token, "cd");
                if (hh != 0 && aa != 0 && redir != 0 && cd != 0) {
                    // -- Collect token into a buffer
                    cmd_buf[i] = token;
                    cmd_buf[i + 1] = NULL;
                    i++;
                }
                // If token is ## or && 
                else {
                    // -- Execute buffer and reset
                    if (hh == 0) {
                        execute_wait(cmd_buf);
                        i = 0;
                        cmd_buf[0] = NULL;
                    }
                    if (aa == 0) {
                        execute_parallel(cmd_buf);
                    }
                    if (redir == 0) {
                        char * filename = strtok(NULL, " \n\t");
                        write_output(cmd_buf, filename);
                        goto HELL;
                    }
                    if (cd == 0) {
                        //printf("Detected cd command\n");
                        token = strtok(NULL, " \n\t");
                        //printf("Moving to: %s\n", token);
                        chdir(token);
                        getcwd(curr_path, PATH_SIZE);
                    }
                    i = 0;
                }
                token = strtok(NULL, " \n\t");
            }
            
            i++;
            cmd_buf[i] = NULL;
            execute_wait(cmd_buf);
            HELL:
            wait(NULL);
        }
    }
}

void handle_sigint() {
    exit(2);
}

void execute_wait(char ** command) {
    int pid = fork();
    //printf("Executable: %s\n", command[0]);
    //printf("Arg 1: %s\n", command[1]);
    if (pid == 0) {
        // Inside child
        // -- Start running said command
        // -- Handle sigint
        signal(SIGTSTP, SIG_DFL);
        signal(SIGINT, SIG_DFL);
        int res = execvp(command[0], &command[0]);
        if (res == -1) {
            printf("Shell: Incorrect command\n", command[0]);
        }
        //printf("Process loading result: %d\n", res);
        exit(0);
    } else if (pid > 0) {
        // Inside parent
        // -- Wait for the child
        int status;
        wait(&status);
        //printf("Wait status: %d\n", status);
    }
}

void execute_parallel(char ** command) {
    int pid = fork();
    if (pid == 0) {
        // Inside child
        // -- Start running said command
        // -- Handler for sigint
        signal(SIGTSTP, SIG_DFL);
        signal(SIGINT, SIG_DFL);
        int res = execvp(command[0], &command[0]);
    } else if (pid > 0) {
        // Inside parent
        // -- Return control to shell
        return;
    }
}

// Maybe unnecesary
void execute_stmt(struct statement s) {
    if (s.command == NULL) {
        if (s.operator == 0) {
            execute_stmt(*s.operands[0]);
            execute_stmt(*s.operands[1]);
        }
    }
}

void execute_multiple_serial(char *** commands) {
    char ** cmd = *commands;
    while(cmd != NULL) {
        execute_wait(cmd);
        cmd++;
    }
}

void execute_multiple_parallel(char *** commands) {
    char **cmd = *commands;
    while(cmd != NULL) {
        execute_parallel(cmd);
        cmd++;
    }
}