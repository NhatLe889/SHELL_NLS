#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

#include "declare_func.h"
#include "looping.h"
#include "nls_command.h"

int main(int argc, char **argv) {
    if (chdir("/Users/nhatle") != 0) {
        perror("starting shell fail at home directory");
        exit(EXIT_FAILURE);
    }

    //Call shell_loop to keep shell running
    shell_loop();

    return EXIT_SUCCESS;
}
