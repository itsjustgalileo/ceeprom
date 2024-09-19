#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 256

// Function to calculate file size
unsigned long get_file_size(FILE *file) {
    fseek(file, 0, SEEK_END);
    unsigned long size = ftell(file);
    fseek(file, 0, SEEK_SET); // Return to start
    return size;
}

// Function to convert a binary string to an integer
long binary_string_to_int(const char *binary_str) {
    long value = 0;
    while (*binary_str) {
        value = (value << 1) | (*binary_str++ - '0');
    }
    return value;
}

// Function to process and convert subsequent values based on their prefix
long process_value(const char *value_str) {
    if (value_str[0] == '$') {
        // Hexadecimal
        return strtol(value_str + 1, NULL, 16);
    } else if (value_str[0] == '%') {
        // Binary
        return binary_string_to_int(value_str + 1);
    } else {
        // Decimal
        return strtol(value_str, NULL, 10);
    }
}

// Function to check if a line is a valid * address line
int is_valid_address_line(const char *line) {
    // Skip leading whitespace
    while (isspace(*line)) line++;

    // Check for * directive
    if (*line != '*') return 0;

    line++; // Skip *

    // Skip whitespace after *
    while (isspace(*line)) line++;

    // Check for $
    if (*line != '$') return 0;

    line++; // Skip $

    // Check if the rest is a valid hex address
    char *end;
    strtol(line, &end, 16);

    // If we couldn't convert anything or if there are invalid characters left
    return *line != '\0' && *end == '\0';
}

// Function to parse command-line arguments manually
int parse_arguments(int argc, char **argv, char **program_name, char **rom_name, unsigned int *startup_address) {
    if (argc < 5) return -1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            if (i + 1 < argc) {
                *program_name = argv[++i];
            } else {
                return -1;
            }
        } else if (strcmp(argv[i], "-r") == 0) {
            if (i + 1 < argc) {
                *rom_name = argv[++i];
            } else {
                return -1;
            }
        } else if (strcmp(argv[i], "-s") == 0) {
            if (i + 1 < argc) {
                *startup_address = (unsigned int)strtol(argv[++i], NULL, 16);
            } else {
                return -1;
            }
        } else {
            return -1;
        }
    }

    return 0;
}

int main(int argc, char **argv) {
    char *program_name = NULL;
    char *rom_name = NULL;
    unsigned int startup_address = 0;

    if (parse_arguments(argc, argv, &program_name, &rom_name, &startup_address) != 0) {
        fprintf(stderr, "Usage: %s -p <program_file> -r <rom_file> -s <startup_address>\n", argv[0]);
        return 1;
    }

    FILE *rom = fopen(rom_name, "r+b");
    if (rom == NULL) {
        perror("Error opening ROM file");
        return 1;
    }

    unsigned long rom_size = get_file_size(rom);

    FILE *program = fopen(program_name, "r");
    if (program == NULL) {
        perror("Error opening program file");
        fclose(rom);
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    unsigned long current_address = startup_address;
    unsigned long line_number = 0;

    fseek(rom, current_address, SEEK_SET);

    while (fgets(line, sizeof(line), program)) {
        line_number++;

        char *comment_start = strchr(line, ';');
        if (comment_start) {
            *comment_start = '\0'; // Ignore comments
        }

        // Trim leading and trailing whitespace
        char *trimmed_line = line;
        while (isspace(*trimmed_line)) trimmed_line++;
        char *end = trimmed_line + strlen(trimmed_line) - 1;
        while (end > trimmed_line && isspace(*end)) end--;
        *(end + 1) = '\0';

        // Skip empty lines
        if (*trimmed_line == '\0') continue;

        // Check for * directive
        if (trimmed_line[0] == '*') {
            if (!is_valid_address_line(trimmed_line)) {
                fprintf(stderr, "Error on line %lu: Invalid address format. Must be preceded by $.\n", line_number);
                fclose(rom);
                fclose(program);
                return 1;
            }

            // Extract address after *
            char *address_str = trimmed_line + 1;
            while (isspace(*address_str)) address_str++; // Skip whitespace

            if (*address_str == '$') {
                unsigned long new_address = (unsigned int)strtol(address_str + 1, NULL, 16);
                if (new_address >= rom_size) {
                    // Adjust address if it exceeds ROM size
                    current_address = new_address - rom_size;
                } else {
                    // Use address as is
                    current_address = new_address;
                }
                
                if (current_address >= rom_size) {
                    fprintf(stderr, "Error on line %lu: Address exceeds ROM size after adjustment.\n", line_number);
                    fclose(rom);
                    fclose(program);
                    return 1;
                }
                
                fseek(rom, current_address, SEEK_SET);
            }
            continue;
        }

        // Process data lines
        char *token = strtok(trimmed_line, " \t\n\r");
        while (token) {
            long value = process_value(token);

            if (current_address >= rom_size) {
                fprintf(stderr, "Error: Program exceeds ROM size.\n");
                fclose(rom);
                fclose(program);
                return 1;
            }

            // Write value to ROM
            fputc(value & 0xFF, rom);
            current_address++;
            if (value > 0xFF) {
                if (current_address >= rom_size) {
                    fprintf(stderr, "Error: Program exceeds ROM size.\n");
                    fclose(rom);
                    fclose(program);
                    return 1;
                }
                fputc((value >> 8) & 0xFF, rom);
                current_address++;
            }

            token = strtok(NULL, " \t\n\r");
        }
    }

    fclose(rom);
    fclose(program);
    return 0;
}
