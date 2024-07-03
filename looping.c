#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

#include "declare_func.h"
#include "looping.h"
#include "nls_command.h"

int shell_loop() {
    char *process_command;
    char **parse_input;
    char **left_half;
    char **right_half;
    char **glob_list;
    int status = 1;
    do {
        int pipe_signal = 0;
        int pipe_position = 0;
        int redir_signal = 0;
        int redir_position = 0;
        int globbing_signal = 0;
        int glob_word_position = 0;

        printf("%% ");
        process_command = shell_initilization();
        parse_input = shell_parse(process_command, &pipe_signal, &pipe_position, &redir_signal, &redir_position, &globbing_signal, &glob_word_position);
        if (pipe_signal == 1) {
            split_operators(parse_input, pipe_position, &left_half, &right_half);
            status = support_piping(left_half, right_half);
        } else if (redir_signal > 0) {
            split_operators(parse_input, redir_position, &left_half, &right_half);
            status = support_redir(left_half, right_half, redir_signal);
        } else if (globbing_signal > 0) {
            int glob_word_count = find_glob_pattern(parse_input[glob_word_position], &glob_list);

            int new_count = glob_word_position + glob_word_count;
            char **new_parse_input = (char **) malloc((new_count + 1) * sizeof(char *));
            for (int i = 0; i < glob_word_position; i++) {
                new_parse_input[i] = strdup(parse_input[i]);
            }
            for (int i = 0; i < glob_word_count; i++) {
                new_parse_input[glob_word_position + i] = strdup(glob_list[i]);
            }
            new_parse_input[new_count] = NULL;

            free_word_list(glob_list);
            
            status = support_shell_command(new_parse_input);

            free_word_list(new_parse_input);

        } else {
            status = support_shell_command(parse_input);
        }

        free_word_list(parse_input);
        free(process_command);

    } while (status);

    return EXIT_SUCCESS;
}