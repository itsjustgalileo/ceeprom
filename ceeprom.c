/**
 * \file        perom.c
 * \brief       romeo generated roms programmer
 * \description this program is technically a virtual minipro clone it
 * take programs listings, text file with instructions written in hex
 * and places them in the rom starting at the specified address
 * */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *program_name = NULL;
char *rom_name = NULL;
unsigned int startup_address = 0; // Added startup address variable.

int main(int argc, char **argv) {
    if(argc < 5) {
        perror("Invalid arguments");
        return 1;
    }

    for(int i = 0; i < argc; ++i) {
        if(strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--program") == 0) {
            program_name = argv[i + 1];
        } else if(strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--rom") == 0) {
            rom_name = argv[i + 1];
        } else if(strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--startup") == 0) { // Parse startup address.
            startup_address = (unsigned int)strtol(argv[i + 1], NULL, 16);
        }
    }
    
    FILE *rom = fopen(rom_name, "r+b");
    if (rom == NULL) {
        return 1;
    }

    fseek(rom, startup_address, SEEK_SET); // Seek to the startup address.

    FILE *program = fopen(program_name, "r");
    if (program == NULL) {
        perror("Error opening source file");
        fclose(rom);
        return 1;
    }

    char line[256];
    while (fgets(line, sizeof(line), program)) {
        char* comment_start = strchr(line, ';');
        if (comment_start) {
            *comment_start = '\0'; // Ignore comments.
        }

        char *token = strtok(line, " \t\n\r");
        while (token) {
            long value = strtol(token, NULL, 16);

            fputc(value & 0xFF, rom); // Write low byte first.
            if (value > 0xFF) {
                fputc((value >> 8) & 0xFF, rom); // Write high byte next if needed.
            }
            token = strtok(NULL, " \t\n\r");
        }
    }

    fclose(rom);
    fclose(program);
    return 0;
}
