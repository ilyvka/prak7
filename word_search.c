#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_PATH_LEN 1024
#define MAX_LINE_LEN 4096

int is_text_file(const char* filename) {
    const char* ext = strrchr(filename, '.');
    if (ext != NULL) {
        const char* text_exts[] = { ".txt", ".c", ".h", ".cpp", ".hpp", ".java",
                                  ".py", ".sh", ".html", ".css", ".js", ".php",
                                  ".md", ".json", ".xml", ".csv", NULL };
        for (int i = 0; text_exts[i] != NULL; i++) {
            if (strcasecmp(ext, text_exts[i]) == 0) {
                return 1;
            }
        }
    }
    // Если расширения нет или оно не в списке, проверим содержимое файла
    FILE* file = fopen(filename, "r");
    if (file == NULL) return 0;

    int ch;
    while ((ch = fgetc(file)) != EOF) {
        if (!isprint(ch) && !isspace(ch)) {
            fclose(file);
            return 0;
        }
    }

    fclose(file);
    return 1;
}

void search_in_file(const char* filename, const char* search_word) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("fopen");
        return;
    }

    char line[MAX_LINE_LEN];
    int line_num = 1;
    int found = 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        if (strstr(line, search_word) != NULL) {
            if (!found) {
                printf("\nFound in file: %s\n", filename);
                found = 1;
            }
            printf("  Line %d: %s", line_num, line);
        }
        line_num++;
    }

    fclose(file);
}

void search_directory(const char* dir_path, const char* search_word) {
    DIR* dir = opendir(dir_path);
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    struct dirent* entry;
    struct stat stat_buf;
    char full_path[MAX_PATH_LEN];

    while ((entry = readdir(dir)) != NULL) {
        // Пропускаем "." и ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        if (lstat(full_path, &stat_buf) == -1) {
            perror("lstat");
            continue;
        }

        if (S_ISDIR(stat_buf.st_mode)) {
            // Рекурсивный поиск в поддиректориях
            search_directory(full_path, search_word);
        }
        else if (S_ISREG(stat_buf.st_mode) && is_text_file(full_path)) {
            search_in_file(full_path, search_word);
        }
    }

    closedir(dir);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <search_word>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char* home_dir = getenv("HOME");
    if (home_dir == NULL) {
        fprintf(stderr, "Could not determine home directory\n");
        return EXIT_FAILURE;
    }

    char target_dir[MAX_PATH_LEN];
    snprintf(target_dir, sizeof(target_dir), "%s/files", home_dir);

    printf("Searching for word '%s' in directory: %s\n", argv[1], target_dir);
    search_directory(target_dir, argv[1]);

    return EXIT_SUCCESS;
}
