#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

#include "declare_func.h"
#include "looping.h"
#include "nls_command.h"

#define STREAM_BUFFER 1024
#define TOKEN_BUFFER 64
#define TOKEN_DELIMITERS " \a\t\r\n"

char* shell_initilization() {
    int input_size = STREAM_BUFFER;
    char* input_command = (char*) malloc(sizeof(char) * input_size);
    if (input_command == NULL) {
        printf("System Command Memory Allocation Fail: malloc");
        exit(EXIT_FAILURE);
    }

    int counter = 0;
    int get_char;
    while ((get_char = getchar()) != '\n' && get_char != EOF) {
        if (counter >= input_size - 1) {
            char* new_input_command = (char*) realloc(input_command, sizeof(char) * (input_size *= 2));
            if (new_input_command == NULL) {
                printf("System Command Memory Allocation Fail: realloc");
                free(input_command);
                exit(EXIT_FAILURE);
            } else {
                input_command = new_input_command;
            }
        }
        input_command[counter] = get_char;
        counter++;
    }
    input_command[counter] = '\0';
    
    return input_command;
}

char** shell_parse(char* process_command) {
    const char delim[] = TOKEN_DELIMITERS;
    int token_buffer_size = TOKEN_BUFFER;
    char *temp_token;
    char **parse_input = (char**) malloc(sizeof(char *) * token_buffer_size);
    if (parse_input == NULL) {
        printf("System Parse Memory Allocation Fail: malloc");
        exit(EXIT_FAILURE);
    }
    temp_token = strtok(process_command, delim);
    int counter = 0;
    while (temp_token != NULL) {
        if (counter >= token_buffer_size - 1) {
            char **new_parse_input = (char**) realloc(parse_input, sizeof(char *) * (token_buffer_size * 2));
            if (new_parse_input == NULL) {
                printf("System Parse Memory Allocation Fail: realloc");
                free(parse_input);
                exit(EXIT_FAILURE);
            } else {
                parse_input = new_parse_input;
            }
        }
        parse_input[counter] = temp_token;
        temp_token = strtok(NULL, delim);
        counter++;
    }
    parse_input[counter] = NULL;

    return parse_input;
}

int shell_starting(char** parse_input) {
    pid_t child_proc_pid = fork();
    int child_status;
    if (child_proc_pid < 0) {
        perror("Fork process failed");
        exit(EXIT_FAILURE);
    } else if (child_proc_pid == 0) {
        if (execvp(parse_input[0], parse_input) == -1) {
            perror("Invalid Command");
        }
    } else {
        do {
            waitpid(child_proc_pid, &child_status, WUNTRACED);
        } while (!WIFEXITED(child_status) && !WIFSIGNALED(child_status));
    }
    return 1;
}

int support_shell_command(char** parse_input) {
    if (parse_input[0] == NULL) {
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_command(); i++) {
        if (strcmp(parse_input[0], command_list[i]) == 0) {
            return (*command_func[i])(parse_input);
        }
    }
    return shell_starting(parse_input);
}