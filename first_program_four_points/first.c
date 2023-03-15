#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

// Метод для подсчёта кол-ва гласных букв
int countVowels(char *string) {
    char *vowels = "AaEeIiOoUuYy";
    int count = 0;
    for (int i = 0; i < strlen(string); ++i) {
        for (int j = 0; j < strlen(vowels); ++j) {
            if (string[i] == vowels[j]) {
                count += 1;
                break;
            }
        }
    }
    return count;
}

// Метод для подсчёта кол-ва согласных букв
int countConsonants(char *string) {
    char *consonants = "BbCcDdFfGgHhJjKkLlMmNnPpQqRrSsTtVvWwXxZz";
    int count = 0;
    for (int i = 0; i < strlen(string); ++i) {
        for (int j = 0; j < strlen(consonants); ++j) {
            if (string[i] == consonants[j]) {
                count += 1;
                break;
            }
        }
    }
    return count;
}
int main(int argc, char *argv[]) {
    char *read_from = argv[1]; // Файл, из которого будет читаться строка.
    char *write_to = argv[2]; // Файл, в который будет записываться итоговый результат.
    char *write_string = (char *) calloc(5000, sizeof(char)); // Для записи в pipe.
    char *read_string = (char *) calloc(5000, sizeof(char)); // Для чтения из pipe.
    int fd[2], input, size_read, size_write, output;
    pid_t parent, child, grandson; // Для создания дочерних потоков.
    if (pipe(fd) < 0) { // Пытаюсь создать канал pipe.
        printf("Can't create pipe parent - child\n");
        exit(-1);
    }
    child = fork(); // Создаю первый дочерний процесс
    if (child == -1) {
        printf("Can't create child process\n");
    } else if (child > 0) { // Вошли в родительский процесс
        input = open(read_from, O_RDONLY); // Открываем файл для чтения строки.
        size_read = read(input, write_string, 5000); // Читаем строку в write_string.
        if (size_read < 0) {
            printf("Parent can't read string from file");
            exit(-1);
        }
        write_string[size_read] = '\0';
        close(fd[0]);
        size_write = write(fd[1], write_string, strlen(write_string)); // Записываем информацию в pipe.
        if (size_write != strlen(write_string)) {
            printf("Can't write all string to pipe\n");
            exit(-1);
        }
        close(fd[1]);
        printf("Parent passed data to child using pipe\n");
    } else if (child == 0) { // Зашли в дочерний процесс
        close(fd[1]);
        size_read = read(fd[0], read_string, 5000); // Из pipe достали строку для обработки.
        if (size_read < 0) {
            printf("Child can't read a string using pipe\n");
            exit(-1);
        }
        close(fd[0]);
        // Создание переменных для красивого вывода в дальнейшем.
        char *first = (char *) calloc(5000, sizeof(char));
        char *second = (char *) calloc(5000, sizeof(char));
        char* vowels = (char *) calloc(20, sizeof(char));
        char *consonants = (char *) calloc(20, sizeof(char));
        // Перекидываю строки в выделенные куски памяти.
        sprintf(vowels, "%s", "Number of vowels: ");
        sprintf(consonants, "%s", "\nNumber of consonants: ");
        sprintf(first, "%d", countVowels(read_string)); // Вызов метода для подсчёта кол-ва гласных.
        sprintf(second, "%d", countConsonants(read_string)); // Вызов метода для подсчёта кол-ва согласных.
        // Объединяю строки.
        strcat(vowels, first);
        strcat(vowels, consonants);
        strcat(vowels, second);
        int new_fd[2];
        // Создаю новый pipe, через него будут общаться child и grandson.
        if (pipe(new_fd) < 0) {
            printf("Can't create pipe child - grandson");
        }
        grandson = fork(); // Создала дочерний процесс от дочернего.
        if (grandson > 0) { // Если сейчас работает child
            close(new_fd[0]);
            size_write = write(fd[1], vowels, strlen(vowels)); // Пишем информацию о кол-ве гласных и согласных.
            if (size_write != strlen(vowels)) {
                printf("Child can't write the number of vowels and consonants\n");
                exit(-1);
            }
            printf("Child has written the number of vowels and consonants\n");
            close(new_fd[1]);
        } else if (grandson == 0) { // Если сейчас работает grandson.
            close(new_fd[1]);
            char *result = (char *) calloc(5000, sizeof(char));
            size_read = read(new_fd[0], result, 5000); // Процесс через pipe читает информацию о гласных и согласных.
            if (size_read < 0) {
                printf("Grandson can't read using a pipe\n");
                exit(-1);
            }
            output = open(write_to, O_CREAT | O_WRONLY, 0666); // Открытие файла для записи.
            size_write = write(output, result, strlen(result)); // Пишем сколько гласных, сколько согласных.
            if (size_write != strlen(result)) {
                printf("Grandson can't put info into file\n");
                exit(-1);
            }
            close(new_fd[0]);
            printf("Grandson put info in file");
        }
    }
}

