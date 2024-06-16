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
    int status = 1;
    do {
        printf("%% ");
        process_command = shell_initilization();
        parse_input = shell_parse(process_command);
        status = support_shell_command(parse_input);

    } while (status);
    /*
    printf("%% ");
    process_command = shell_initilization();
    parse_input = shell_parse(process_command);

    int i = 0;
    while (parse_input[i] != NULL) {
        printf("%s\n", parse_input[i]);
        i++;
    }
    */

    free(process_command);

    return EXIT_SUCCESS;
}