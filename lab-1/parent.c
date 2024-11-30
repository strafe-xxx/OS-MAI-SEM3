#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

#define BUFFER_SIZE 256

void handle_error(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

int main() {
    int pipe1[2], pipe2[2];
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        handle_error("Ошибка при создании каналов");
    }

    char file1_name[BUFFER_SIZE], file2_name[BUFFER_SIZE];
    printf("Введите имя файла для записи вывода child1: ");
    fflush(stdout);
    if (!fgets(file1_name, BUFFER_SIZE, stdin)) {
        handle_error("Ошибка при чтении имени файла");
    }
    file1_name[strcspn(file1_name, "\n")] = '\0';

    printf("Введите имя файла для записи вывода child2: ");
    fflush(stdout);
    if (!fgets(file2_name, BUFFER_SIZE, stdin)) {
        handle_error("Ошибка при чтении имени файла");
    }
    file2_name[strcspn(file2_name, "\n")] = '\0';

    printf("Создаём первый дочерний процесс...\n");
    pid_t child1 = fork();
    if (child1 == -1) {
        handle_error("Ошибка при fork для child1");
    }
    if (child1 == 0) {
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);

        int file1_fd = open(file1_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (file1_fd == -1) {
            handle_error("Ошибка при открытии файла для child1");
        }
        dup2(pipe1[0], STDIN_FILENO);
        dup2(file1_fd, STDOUT_FILENO);
        close(file1_fd);
        close(pipe1[0]);

        execl("./child1", "./child1", NULL);
        handle_error("Ошибка при execl для child1");
    }

    printf("Создаём второй дочерний процесс...\n");
    pid_t child2 = fork();
    if (child2 == -1) {
        handle_error("Ошибка при fork для child2");
    }
    if (child2 == 0) {
        close(pipe2[1]);
        close(pipe1[0]);
        close(pipe1[1]);

        int file2_fd = open(file2_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (file2_fd == -1) {
            handle_error("Ошибка при открытии файла для child2");
        }
        dup2(pipe2[0], STDIN_FILENO);
        dup2(file2_fd, STDOUT_FILENO);
        close(file2_fd);
        close(pipe2[0]);

        execl("./child2", "./child2", NULL);
        handle_error("Ошибка при execl для child2");
    }

    close(pipe1[0]);
    close(pipe2[0]);

    char input[BUFFER_SIZE];
    srand(time(NULL));

    printf("Введите строки для обработки (напишите 'exit' для завершения):\n");
    while (1) {
        if (!fgets(input, BUFFER_SIZE, stdin)) {
            handle_error("Ошибка чтения строки");
        }
        input[strcspn(input, "\n")] = '\0';
        if (strcmp(input, "exit") == 0) break;

        if ((rand() % 100) < 80) {
            printf("Отправляем в pipe1: %s\n", input);
            if (write(pipe1[1], input, strlen(input) + 1) == -1) {
                handle_error("Ошибка записи в pipe1");
            }
        } else {
            printf("Отправляем в pipe2: %s\n", input);
            if (write(pipe2[1], input, strlen(input) + 1) == -1) {
                handle_error("Ошибка записи в pipe2");
            }
        }
    }

    close(pipe1[1]);
    close(pipe2[1]);

    wait(NULL);
    wait(NULL);

    printf("Родительский процесс завершён.\n");
    return 0;
}
