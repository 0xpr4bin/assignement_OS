#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>

void copy_file(const char *source, const char *destination) {
    int src_fd = open(source, O_RDONLY);
    if (src_fd < 0) {
        perror("Failed to open source file");
        exit(EXIT_FAILURE);
    }

    int dest_fd = open(destination, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd < 0) {
        perror("Failed to open destination file");
        close(src_fd);
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    ssize_t bytes_read;

    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {
        if (write(dest_fd, buffer, bytes_read) != bytes_read) {
            perror("Error writing to destination file");
            close(src_fd);
            close(dest_fd);
            exit(EXIT_FAILURE);
        }
    }

    close(src_fd);
    close(dest_fd);
}

void copy_directory(const char *source_dir, const char *dest_dir) {
    mkdir(dest_dir, 0755);

    DIR *dir = opendir(source_dir);
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char source_path[1024], dest_path[1024];
        snprintf(source_path, sizeof(source_path), "%s/%s", source_dir, entry->d_name);
        snprintf(dest_path, sizeof(dest_path), "%s/%s", dest_dir, entry->d_name);

        if (entry->d_type == DT_DIR) {
            copy_directory(source_path, dest_path);
        } else {
            copy_file(source_path, dest_path);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source> <destination>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *source = argv[1];
    const char *destination = argv[2];

    struct stat st;
    if (stat(source, &st) == -1) {
        perror("Failed to stat source");
        exit(EXIT_FAILURE);
    }

    if (S_ISDIR(st.st_mode)) {
        struct stat dest_stat;
        if (stat(destination, &dest_stat) == -1) {
            mkdir(destination, 0755);
        } else if (!S_ISDIR(dest_stat.st_mode)) {
            fprintf(stderr, "Destination is not a directory\n");
            exit(EXIT_FAILURE);
        }
        copy_directory(source, destination);
    } else {
        struct stat dest_stat;
        if (stat(destination, &dest_stat) == 0 && S_ISDIR(dest_stat.st_mode)) {
            char dest_file[1024];
            snprintf(dest_file, sizeof(dest_file), "%s/%s", destination, strrchr(source, '/') ? strrchr(source, '/') + 1 : source);
            copy_file(source, dest_file);
        } else {
            copy_file(source, destination);
        }
    }

    return 0;
}
