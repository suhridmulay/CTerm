#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

#define EXIT_CODE "quit"
#define PATH_SIZE 256

#define PROMPT ">>>"

char curr_path[PATH_SIZE];

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
            printf("You wanted to run: %s\n", linebuf);
        }
    }
}