#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <stdbool.h>

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
    int curr_token_size = TOKEN_BUFFER;

    bool within_single_quote = false;
    bool within_double_quote = false;

    char **parse_input = (char**) malloc(sizeof(char *) * token_buffer_size);
    if (parse_input == NULL) {
        printf("System Parse Memory Allocation Fail: malloc");
        exit(EXIT_FAILURE);
    }

    char *curr_token = (char*) malloc(sizeof(char) * curr_token_size);
    if (curr_token == NULL) {
        printf("System Token Memory Allocation Fail: malloc");
        free(parse_input);
        exit(EXIT_FAILURE);
    }

    int count_curr = 0;
    int count_parse = 0;
    for (int i = 0; process_command[i] != '\0'; i++) {
        char character = process_command[i];
        bool prev_flag;

        if (count_parse >= token_buffer_size - 1) {
            char **new_parse_input = (char**) realloc(parse_input, sizeof(char *) * (token_buffer_size * 2));
            if (new_parse_input == NULL) {
                printf("System Parse Memory Allocation Fail: realloc");
                free(curr_token);
                free(parse_input);
                exit(EXIT_FAILURE);
            } else {
                parse_input = new_parse_input;
            }
            token_buffer_size *= 2;
        }

        if (count_curr >= curr_token_size - 1) {
            char *new_curr_token = (char*) realloc(curr_token, sizeof(char) * (curr_token_size * 2));
            if (new_curr_token == NULL) {
                printf("System Parse Memory Allocation Fail: realloc");
                free(curr_token);
                free(parse_input);
                exit(EXIT_FAILURE);
            } else {
                curr_token = new_curr_token;
            }
            curr_token_size *= 2;
        }

        if (character == ' ' && within_double_quote == false && within_single_quote == false) {
            if (count_curr > 0) {
                curr_token[count_curr] = '\0';
                parse_input[count_parse] = strdup(curr_token);
                count_parse++;
                curr_token[0] = '\0';
                count_curr = 0;
            }

        } else if (character == '\'' && within_double_quote == false) {
            prev_flag = within_single_quote;
            within_single_quote = !within_single_quote;

            if (prev_flag == true && within_double_quote == false) {
                curr_token[count_curr] = '\0';
                parse_input[count_parse] = strdup(curr_token);
                count_parse++;
                curr_token[0] = '\0';
                count_curr = 0;
            }

        } else if (character == '"' && within_single_quote == false) {
            prev_flag = within_double_quote;
            within_double_quote = !within_double_quote;

            if (prev_flag == true && within_double_quote == false) {
                curr_token[count_curr] = '\0';
                parse_input[count_parse] = strdup(curr_token);
                count_parse++;
                curr_token[0] = '\0';
                count_curr = 0;
            }

        } else if (character == '\\' && process_command[i + 1] != '\0') {
            char next_character = process_command[++i];
            if (within_single_quote || within_double_quote) {
                if (next_character == 't') {
                    curr_token[count_curr++] = '\t';
                } else if (next_character == 'a') {
                    curr_token[count_curr++] = '\a';
                } else if (next_character == 'r') {
                    curr_token[count_curr++] = '\r';
                } else if (next_character == 'n') {
                    curr_token[count_curr++] = '\n';
                } else {
                    curr_token[count_curr++] = next_character;
                }
            } else {
                curr_token[count_curr++] = next_character;
            }

        } else if (strchr(delim, character) != NULL) {
            if (within_single_quote || within_double_quote) {
                curr_token[count_curr++] = character;
            } else {
                if (count_curr > 0) {
                    curr_token[count_curr] = '\0';
                    parse_input[count_parse++] = strdup(curr_token);
                    curr_token[0] = '\0';
                    count_curr = 0; 
                }
            }
        } else {
            curr_token[count_curr] = character;
            count_curr++;
        }

    }
    if (count_curr > 0) {
        curr_token[count_curr] = '\0';
        parse_input[count_parse++] = strdup(curr_token);
    }

    parse_input[count_parse] = NULL;
    free(curr_token);

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