#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void to_lower(char *str) {
    while (*str) {
        *str = tolower(*str);
        str++;
    }
}

int case_insensitive_match(const char *a, const char *b) {
    char *a_lower = strdup(a);
    char *b_lower = strdup(b);
    to_lower(a_lower);
    to_lower(b_lower);
    int result = strcmp(a_lower, b_lower) == 0;
    free(a_lower);
    free(b_lower);
    return result;
}

void search_directory(const char *searchpath, const char *filename, int recursive, int case_insensitive) {
    DIR *dir = opendir(searchpath);
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", searchpath, entry->d_name);

        if (entry->d_type == DT_DIR && recursive) {
            // Recur into subdirectories
            pid_t pid = fork();
            if (pid == 0) {
                search_directory(fullpath, filename, recursive, case_insensitive);
                exit(0);
            } else if (pid > 0) {
                wait(NULL); // Prevent zombie processes
            }
        } else if (entry->d_type == DT_REG) {
            int match = case_insensitive ? case_insensitive_match(entry->d_name, filename) : strcmp(entry->d_name, filename) == 0;
            if (match) {
                printf("%d: %s: %s\n", getpid(), filename, fullpath);
            }
        }
    }
    closedir(dir);
}

int main(int argc, char *argv[]) {
    int recursive = 0;
    int case_insensitive = 0;
    int opt;
    char *searchpath = NULL;

    // Argument parsing
    while ((opt = getopt(argc, argv, "Ri")) != -1) {
        switch (opt) {
            case 'R':
                recursive = 1;
                break;
            case 'i':
                case_insensitive = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-R] [-i] <searchpath> <filename1> [filename2] ...\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Expected searchpath and filenames\n");
        exit(EXIT_FAILURE);
    }

    searchpath = argv[optind++];
    if (optind >= argc) {
        fprintf(stderr, "Expected at least one filename to search\n");
        exit(EXIT_FAILURE);
    }

    // For each filename, create a child process to search
    for (int i = optind; i < argc; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            search_directory(searchpath, argv[i], recursive, case_insensitive);
            exit(0);
        } else if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all children to finish
    while (wait(NULL) > 0);
    
    return 0;
}
