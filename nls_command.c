#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "declare_func.h"
#include "looping.h"
#include "nls_command.h"

char *command_list[] = {
    "exit",
    "help",
    "cd",
    "pwd",
    "ls",
    "mkdir"
};

int (*command_func[])(char **) = {
    &command_exit,
    &command_help,
    &command_cd,
    &command_pwd,
    &command_ls,
    &command_mkdir
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

int command_ls(char **parse_input) {
    DIR *curr_dir;
    struct dirent *storage_dir;
    const char *start_path = ".";

    curr_dir = opendir(start_path);
    if (curr_dir == NULL) {
        perror("accessing directory error");
        exit(EXIT_FAILURE);
    }

    while ((storage_dir = readdir(curr_dir)) != NULL) {
        if (strcmp(storage_dir->d_name, ".") == 0 || strcmp(storage_dir->d_name, "..") == 0) {
            continue;
        }
        //skip the hidden file that are in the directory
        if (storage_dir->d_name[0] == '.') {
            continue;
        }
        printf("%s\n\n", storage_dir->d_name);
    }

    closedir(curr_dir);

    return 1;
}

int command_mkdir(char **parse_input) {
    // permission mode is 0755 (rwxr-xr-x)
    mode_t permis_mode = 0755;

    if (mkdir(parse_input[1], permis_mode) == -1) {
        switch (errno) {
            case EEXIST:
                fprintf(stderr, "mkdir: cannot create directory '%s': File exists\n", parse_input[1]);
                break;
            case ENOENT:
                fprintf(stderr, "mkdir: cannot create directory '%s': No such file or directory\n", parse_input[1]);
                break;
            case EACCES:
                fprintf(stderr, "mkdir: cannot create directory '%s': Permission denied\n", parse_input[1]);
                break;
            default:
                fprintf(stderr, "mkdir: cannot create directory '%s': Unknown error\n", parse_input[1]);
                break;
        }
        exit(EXIT_FAILURE);
    } else {
        printf("Success! New Directory: /%s\n", parse_input[1]);
    }

    return 1;
}