#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

#include "declare_func.h"
#include "looping.h"
#include "nls_command.h"

int main(int argc, char **argv) {
    //Call shell_loop to keep shell running
    shell_loop();

    return EXIT_SUCCESS;
}
