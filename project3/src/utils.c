#include "../include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void read_config_file(const char *config_file, char *command_file, char *text_file) {
    FILE *file = fopen(config_file, "r");
    if (!file) {
        perror("Failed to open config file");
        exit(EXIT_FAILURE);
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Skip empty lines or lines starting with a comment symbol
        if (line[0] == '\n' || line[0] == '\r' || line[0] == '#') {
            continue;
        }

        char *equal_sign = strchr(line, '=');
        if (equal_sign) {
            // Split the key and value
            *equal_sign = '\0';
            char *key = line;
            char *value = equal_sign + 1;

            // Trim leading/trailing spaces from key
            while (*key == ' ') key++;
            char *key_end = key + strlen(key) - 1;
            while (key_end > key && (*key_end == ' ')) {
                *key_end = '\0';
                key_end--;
            }

            // Trim leading/trailing spaces from value
            while (*value == ' ') value++;
            char *value_end = value + strlen(value) - 1;
            while (value_end > value && (*value_end == ' ' || *value_end == '\n')) {
                *value_end = '\0';
                value_end--;
            }

            // Assign value to the correct variable
            if (strcmp(key, "COMMAND_TEXT") == 0) {
                strcpy(command_file, value);
            } else if (strcmp(key, "LARGE_TEXT") == 0) {
                strcpy(text_file, value);
            }
        }
    }

    fclose(file);
}

// This is how it should work out
// void read_config_file(const char *config_file, char *command_file, char *text_file) {
//     FILE *file = fopen(config_file, "r");
//     if (!file) {
//         perror("Failed to open config file");
//         exit(EXIT_FAILURE);
//     }

//     char line[256];
//     while (fgets(line, sizeof(line), file)) {
//         char *key = strtok(line, "=");
//         char *value = strtok(NULL, "\n");

//         if (key && value) {
//             // Remove potential leading/trailing spaces
//             while (*value == ' ') value++; // Trim leading spaces
//             if (strcmp(key, "COMMAND_TEXT") == 0) {
//                 strcpy(command_file, value);
//             } else if (strcmp(key, "LARGE_TEXT") == 0) {
//                 strcpy(text_file, value);
//             }
//         }
//     }

//     fclose(file);
// }
