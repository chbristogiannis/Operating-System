#include "../include/utils.h"
#include "../include/definitions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char command_file[256];
    char text_file[256];

    read_config_file("config.cfg", command_file, text_file);

    printf("Command file: %s\n", command_file);
    printf("Text file: %s\n", text_file);

    // Call the parent process function
    parent_process(command_file, text_file);

    // Read instructions from the command file
    // char com[256] = "./test_files/config_3_100.txt";
    // FILE *cf = fopen(command_file, "r");
    // if (!cf) {
    //     perror("Failed to open command file");
    //     exit(EXIT_FAILURE);
    // }
    // fclose(cf);

    return 0;
}
