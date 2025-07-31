/*
 * Program Briefing :
 *  It takes in a file with .RID extension and converts it to .bmp image.
 *  .RID file comes from Hardik Kalasua OV7670 modified code made for Arduino Uno later modified by me to work on Arduino Mega 2560.
 *  .RID = RAW IMAGE DATA
 *  It takes in the data, Pastes a bmp image header and outputs the image as .bmp file.
 *  It will take a directory path which contains the .RID files and outputs the processed image on other directory.
 *
 *  Steps:
 *   1) Program is executed from cmd and takes in the necessary arguments.
 *   2) It then starts the conversion process file by file and outputs a new image to the desired directory.
 *   3) The name of the output will be : output_<original_name>.bmp
 *
 *  Rules:
 *   1) It just searches for the file with valid extension aka .RID
 *   2) It does not scan the file to see if it's valid or not.
 *   3) If you are not able to open the .bmp it then its most likely that the .RID file was corrupted.
 *   4) Only the absolute path of directory will be processed.
 *   5) Program must not run from the .exe file directly as it may not be able to find any arguments.
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>

#define PATH_MAX 256

const unsigned char bmp_header[54] = {
    0x42, 0x4D,
    0xDE, 0x82, 0x03, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x36, 0x00, 0x00, 0x00,
    0x28, 0x00, 0x00, 0x00,
    0x40, 0x01, 0x00, 0x00,
    0xF0, 0x00, 0x00, 0x00,
    0x01, 0x00,
    0x18, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x82, 0x03, 0x00,
    0x13, 0x0B, 0x00, 0x00,
    0x13, 0x0B, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};


char* get_substring(const char* string, unsigned int start_index, unsigned int last_index) {
    if (string == NULL) return NULL;

    char* result = (char*) calloc(last_index - start_index + 1, sizeof(char));
    unsigned int byte_counter = 0;
    for (unsigned int i = start_index; i < last_index; i++) {
        result[byte_counter++] = string[i];
    }
    result[byte_counter] = '\0';

    return result;
}


int main(int argv, char* argc[]) {
    if (argv < 3) { fprintf(stderr, "Information given is not enough."); return 1; }

    DIR* inDIR = opendir(argc[1]);
    if (inDIR == NULL) { fprintf(stderr, "Error opening directory\n"); return 1; }

    // Check if DIR is present where we have to output the file.
    DIR* outDIR = opendir(argc[2]);
    if (outDIR == NULL) { fprintf(stderr, "Error opening directory\n"); return 1; }
    closedir(outDIR);

    struct dirent* current_file;
    while ((current_file = readdir(inDIR)) != NULL) {
        char* abs_path = calloc(strlen(argc[1]) + current_file->d_namlen + 5, 1);

        strcat(abs_path, argc[1]);
        strcat(abs_path, "\\");
        strcat(abs_path, current_file->d_name);
        unsigned int abs_size = strlen(abs_path);

        if (abs_size >= 4) {
            char* substr = get_substring(abs_path, abs_size - 4, abs_size);


            if (strcmp(substr, ".RID") == 0)
            {
                char WRITE_FILE_NAME[PATH_MAX] = "";
                char* current_file_name = get_substring(current_file->d_name, 0, current_file->d_namlen - 4);

                snprintf(WRITE_FILE_NAME, PATH_MAX, "%s\\%s.bmp", argc[2], current_file_name);
                printf("%s\n", WRITE_FILE_NAME);

                FILE* out = fopen(WRITE_FILE_NAME, "wb");
                FILE* in = fopen(abs_path, "rb");

                fwrite(bmp_header, 1, 54, out);

                int byte;
                while ((byte = fgetc(in)) != EOF) {
                    for (int i = 0; i < 3; ++i) fputc(byte, out);
                }

                fclose(out);
                fclose(in);

                free(current_file_name);
            }


            free(substr);
        }

        free(abs_path);
    }

    closedir(inDIR);
    return 0;
}