#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <limits.h>

#include "declare_func.h"
#include "looping.h"
#include "nls_command.h"

char *command_list[] = {
    "exit",
    "help",
    "cd",
    "pwd"
};

int (*command_func[])(char **) = {
    &command_exit,
    &command_help,
    &command_cd,
    &command_pwd
};

int num_command() {
    return (sizeof(command_list)/sizeof(char*));
}

// command implementation down below!

int command_cd(char **parse_input) {
    if (parse_input[1] == NULL) {
        printf("Expected argument for \"cd\"\n");
    } else {
        if (chdir(parse_input[1]) != 0) {
            perror("Incorrect path directory");
        }
    }
    return 1;
}

int command_exit(char **parse_input) {
    exit(EXIT_SUCCESS);
}

int command_help(char **parse_input) {
    printf("The available three function are 'cd', 'exit', 'help'.\n");
    printf("Format: COMMAND;DIRECTORY\n");
    printf("current build in command:\n");

    for (int i = 0; i < num_command(); i++) {
        printf("    %s\n", command_list[i]);
    }
    return 1;
}

int command_pwd(char **parse_input) {
    char path[PATH_MAX];
    if (getcwd(path, sizeof(path)) != NULL) {
        printf("%s\n", path);
    } else {
        perror("pwd command fail");
    }
    return 1;
}

