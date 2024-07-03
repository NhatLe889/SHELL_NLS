#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <glob.h>

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

char** shell_parse(char *process_command, int *pipe_signal, int *pipe_position, int *redir_signal, int *redir_position, int *globbing_signal, int *glob_word_position) {
    const char delim[] = TOKEN_DELIMITERS;
    int token_buffer_size = TOKEN_BUFFER;
    int curr_token_size = TOKEN_BUFFER;

    bool within_single_quote = false;
    bool within_double_quote = false;

    bool within_square_brackets_globbing = false;
    bool within_braces_globbing = false;

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

        } else if (character == '|' && within_single_quote == false && within_double_quote == false) {
            *pipe_signal = 1;
            if (count_curr > 0) {
                curr_token[count_curr] = '\0';
                parse_input[count_parse++] = strdup(curr_token);
                curr_token[0] = '\0';
                count_curr = 0; 
            }
            *pipe_position = count_parse;
            count_parse++;

        } else if ((character == '<' || character == '>' || (character == '>' && process_command[i + 1] == '>'))
        && within_single_quote == false && within_double_quote == false) {
            if (character == '>') {
                if (process_command[i + 1] == '>') { 
                    *redir_signal = 2; //output stream redirect '>>' Append to file if file exist
                    i++;
                } else {
                    *redir_signal = 1; //output stream redirect '>' - Overwrite if file exist
                }
            } else {
                *redir_signal = 3; //input stream redirect '<'
            }

            if (count_curr > 0) {
                curr_token[count_curr] = '\0';
                parse_input[count_parse++] = strdup(curr_token);
                curr_token[0] = '\0';
                count_curr = 0; 
            }
            *redir_position = count_parse;
            count_parse++;

        } else if ((character == '*' || character == '?' || character == '[' || character == ']' || character == '{' || character == '}')
        && within_single_quote == false && within_double_quote == false) {
            if (character == '*') {
                *globbing_signal = 1;
                *glob_word_position = count_parse;
            } else if (character == '?') {
                *globbing_signal = 2;
                *glob_word_position = count_parse;
            } else if (character == '[') {
                within_square_brackets_globbing = true;
            } else if (character == '{') {
                within_braces_globbing = true;
            } else {
                if (character == ']' && within_square_brackets_globbing == true) {
                    *globbing_signal = 3;
                    *glob_word_position = count_parse;
                    within_square_brackets_globbing = false;
                } else if (character == '}' && within_braces_globbing == true) {
                    *globbing_signal = 4;
                    *glob_word_position = count_parse;
                    within_braces_globbing = false;
                }
            }
            curr_token[count_curr] = character;
            count_curr++;
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

//spliting parse command into left half and right half based on the piping and rediretion operators position.
void split_operators(char** parse_input, int pipe_position, char*** left_half, char*** right_half) {
    int left_size = pipe_position;
    int right_size = 0;
    for (int i = pipe_position + 1; parse_input[i] != NULL; i++) {
        right_size++;
    }

    *left_half = (char**) malloc(sizeof(char *) * (left_size + 1));
    *right_half = (char**) malloc(sizeof(char *) * (right_size + 1));

    if (left_half == NULL || right_half == NULL) {
        printf("System Token Memory Allocation Fail: malloc");
        free(parse_input);
        //free(curr_token);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < left_size; i++) {
        (*left_half)[i] = strdup(parse_input[i]);
    }
    (*left_half)[left_size] = NULL;

    for (int i = 0; i < right_size; i++) {
        (*right_half)[i] = strdup(parse_input[i + pipe_position + 1]);
    }
    (*right_half)[right_size] = NULL;
}

//function used to execute left and right half of piping command.
int piping_execute(char **left_half, char**right_half) {

    // 0 for read end pipe, 1 for write end pipe
    int pipe_fd[2];
    
    if (pipe(pipe_fd) < 0) {
        perror("piping fail");
        exit(EXIT_FAILURE);
    }

    pid_t child1 = fork();
    if (child1 < 0) {
        perror("child1 fork process fail");
        exit(EXIT_FAILURE);
    } else if (child1 == 0) {
        //during the child1 process
        close(pipe_fd[0]);
        dup2(pipe_fd[1], STDOUT_FILENO);
        close(pipe_fd[1]);

        if (execvp(left_half[0], left_half) == -1) {
            perror("child1 execution process fail");
            exit(EXIT_FAILURE);
        }
    }

    pid_t child2 = fork();
    if (child2 < 0) {
        perror("child2 for process fail");
        exit(EXIT_FAILURE);
    } else if (child2 == 0) {
        //during the child2 process
        close(pipe_fd[1]);
        dup2(pipe_fd[0], STDIN_FILENO);
        close(pipe_fd[0]);

        if (execvp(right_half[0], right_half) == -1) {
            perror("child2 execution process fail");
            exit(EXIT_FAILURE);
        }
    }

    //in the parent process
    close(pipe_fd[0]);
    close(pipe_fd[1]);

    waitpid(child1, NULL, 0); //don't care how the child terminate
    waitpid(child2, NULL, 0); //don't care how the child terminate

    free(left_half);
    free(right_half);

    return 1;
}

//This function used to make the right side into standard I/O and executing the left side command
int redirection_to_file(char **left_half, char **right_half, int redir_signal) {
    pid_t pid_redir = fork();
    if (pid_redir < 0) {
        perror("fork within redirection process fail");
        exit(EXIT_FAILURE);
    } else if (pid_redir == 0) {
        int in_out_fd;
        if (redir_signal == 1) { //output stream redirect '>' - Overwrite if file exist
            in_out_fd = open(right_half[0], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (in_out_fd == -1) {
                perror("output stream redirect '>' failure");
                exit(EXIT_FAILURE);
            }
            dup2(in_out_fd, STDOUT_FILENO);
            close(in_out_fd);
        } else if (redir_signal == 2) { //output stream redirect '>>' Append to file if file exist
            in_out_fd = open(right_half[0], O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (in_out_fd == -1) {
                perror("output stream redirect '>>' failure");
                exit(EXIT_FAILURE);
            }
            dup2(in_out_fd, STDOUT_FILENO);
            close(in_out_fd);
        } else if (redir_signal == 3) { //input stream redirect '<'
            in_out_fd = open(right_half[0], O_RDONLY);
            if (in_out_fd == -1) {
                perror("input stream redirect '<' failure");
                exit(EXIT_FAILURE);
            }
            dup2(in_out_fd, STDIN_FILENO);
            close(in_out_fd);
        }

        //left half executing commands
        if (execvp(left_half[0], left_half) == -1) {
            perror("left side command fail within redirection");
            exit(EXIT_FAILURE);
        }
    } else { //during the parent process to wait for the child to terminte
        int status;
        waitpid(pid_redir, &status, 0); //what to know how the child process terminate
    }

    return 1;
}

//This function used to find the correct command that are executing between left and right command
int support_piping(char **left_half, char**right_half) {
    if (left_half[0] == NULL || right_half[0] == NULL) {
        return 1;
    }

    for (int i = 0; i < num_command(); i++) {
        if (strcmp(left_half[0], command_list[i]) == 0) {
            return (*command_func[i])(left_half);
        }
    }

    for (int i = 0; i < num_command(); i++) {
        if (strcmp(right_half[0], command_list[i]) == 0) {
            return (*command_func[i])(right_half);
        }
    }

    return piping_execute(left_half, right_half);
}

//This function used to find the left side command for direction process
int support_redir(char **left_half, char **right_half, int redir_signal) {
    for (int i = 0; i < num_command(); i++) {
        if (strcmp(left_half[0], command_list[i]) == 0) {
            return (*command_func[i])(left_half);
        }
    }

    return redirection_to_file(left_half, right_half, redir_signal);
}

int find_glob_pattern(char *pattern, char ***pattern_list) {
    glob_t result_globbing;
    memset(&result_globbing, 0, sizeof(result_globbing));

    int check_result = glob(pattern, GLOB_TILDE | GLOB_BRACE | GLOB_NOCHECK | GLOB_MARK, NULL, &result_globbing);
    if (check_result != 0) {
        globfree(&result_globbing);
        exit(EXIT_FAILURE);
    }

    *pattern_list = (char**) malloc((result_globbing.gl_pathc + 1) * sizeof(char *));
    if (*pattern_list == NULL) {
        globfree(&result_globbing);
        perror("malloc allocation fail: globbing process");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < result_globbing.gl_pathc; ++i) {
        (*pattern_list)[i] = strdup(result_globbing.gl_pathv[i]);
        if ((*pattern_list)[i] == NULL) {
            perror("globbing: allocate result list fail");
            exit(EXIT_FAILURE);
        }
    }
    (*pattern_list)[result_globbing.gl_pathc] = NULL;

    //clean up
    globfree(&result_globbing);

    return result_globbing.gl_pathc;
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
        return 1;
    }

    for (int i = 0; i < num_command(); i++) {
        if (strcmp(parse_input[0], command_list[i]) == 0) {
            return (*command_func[i])(parse_input);
        }
    }
    return shell_starting(parse_input);
}

void free_word_list(char **list) {
    for (int i = 0; list[i] != NULL; i++) {
        free(list[i]);
    }
    free(list);
}