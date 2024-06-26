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
    int status = 1;
    do {
        int pipe_signal = 0;
        int pipe_position = 0;
        printf("%% ");
        process_command = shell_initilization();
        parse_input = shell_parse(process_command, &pipe_signal, &pipe_position);
        if (pipe_signal == 1) {
            split_piping(parse_input, pipe_position, &left_half, &right_half);
            status = support_piping(left_half, right_half);
        } else {
            status = support_shell_command(parse_input);
        }

    } while (status);

    free(process_command);

    return EXIT_SUCCESS;
}