#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
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
    char *read_from = argv[1];                               // Файл, из которого будет читаться строка.
    char *write_to = argv[2];                                // Файл, в который будет записываться итоговый результат.
    char *write_string = (char *) calloc(5000, sizeof(char));// Для записи в pipe.
    char *read_string = (char *) calloc(5000, sizeof(char)); // Для чтения из pipe.
    int size_write, size_read, input, output, input_fifo, output_fifo;
    pid_t child_one, child_two;
    int *status;
    // Пытаемся создать child_one.
    child_one = fork();
    if (child_one < 0) {
        printf("Can't create child_one process\n");
    }
    // Зашли в процесс child_one
    if (child_one == 0) {
        char name_fifo[9] = "work.fifo";// Файл для создания fifo.
        // Именованный канал для общения child_one и child_two.
        if (mknod(name_fifo, S_IFIFO | 0666, 0) < 0) {
            printf("Can't create FIFO\n");
            exit(-1);
        }
        // Пытаемся создать ещё один дочерний процесс.
        child_two = fork();
        if (child_two < 0) {
            printf("Can't create child_two process\n");
        }
        // Находимся в первом дочернем процессе (child_one)
        else if (child_two > 0) {
            // Открываем файл для чтения строки для дальнейшей отработки.
            if ((input = open(read_from, O_RDONLY)) < 0) {
                printf("Child_one can't open file to read string for work\n");
            }
            size_read = read(input, write_string, 5000);// Читаем строку в write_string.
            if (size_read < 0) {
                printf("Child_one can't read string from file\n");
                exit(-1);
            }
            // Пытаемся открыть fifo для передачи строки от 1 ребёнка ко 2.
            if ((input_fifo = open(name_fifo, O_WRONLY)) < 0) {
                printf("Child_one can't open FIFO for writing\n");
                perror("oksw");
                exit(-1);
            }
            // Пытаемся записать строку в именованный канал для 2 ребёнка.
            size_write = write(input_fifo, write_string, strlen(write_string));
            if (size_write != strlen(write_string)) {
                printf("Parent can't write to fifo\n");
                exit(-1);
            }
            close(input_fifo);
            printf("Child_one successfully has written string using fifo\n");
        }
        // Заходим во второй дочерний процесс (child_two)
        else {
            sleep(3);
            // Пытаемся открыть fifo для чтения данных от 1 ребёнка
            if ((output_fifo = open(name_fifo, O_RDONLY)) < 0) {
                printf("Child_two can't open fifo for reading\n");
                exit(-1);
            }
            size_read = read(output_fifo, read_string, 5000);
            if (size_read < 0) {
                printf("Child_two can't read from fifo\n");
                exit(-1);
            }
            close(output_fifo);
            // Создание переменных для красивого вывода в дальнейшем.
            char *first = (char *) calloc(5000, sizeof(char));
            char *second = (char *) calloc(5000, sizeof(char));
            char *vowels = (char *) calloc(20, sizeof(char));
            char *consonants = (char *) calloc(20, sizeof(char));
            // Перекидываю строки в выделенные куски памяти.
            sprintf(vowels, "%s", "Number of vowels: ");
            sprintf(consonants, "%s", "\nNumber of consonants: ");
            sprintf(first, "%d", countVowels(read_string));     // Вызов метода для подсчёта кол-ва гласных.
            sprintf(second, "%d", countConsonants(read_string));// Вызов метода для подсчёта кол-ва согласных.
            // Объединяю строки.
            strcat(vowels, first);
            strcat(vowels, consonants);
            strcat(vowels, second);
            // Открываем второй fifo для обмена вида: второй ребёнок отправляет данные первому ребёнку.
            if ((input_fifo = open(name_fifo, O_WRONLY)) < 0) {
                printf("Child_two can't open fifo for writing\n");
            }
            size_write = write(input_fifo, vowels, strlen(vowels));
            if (size_write != strlen(vowels)) {
                printf("Child_two can't write to second fifo\n");
            }
            //сlose(input_fifo);
            printf("Child_two passed result string using second fifo\n");
        }
        // Нам нужно снова оказаться в child_one процессе, чтобы считать обработанный результат.
        // Ждём при помощи sleep.
        // Нам нужно, чтобы child_one дождался пока child_two запишет готовый результат.
        if (child_two > 0) {
            // Ждём child_two.
            sleep(2);
            // Пытаемся открыть второй fifo для чтения итогового ответа.
            if ((output_fifo = open(name_fifo, O_RDONLY)) < 0) {
                printf("Child_one can't open second fifo\n");
            }
            // Пытаемся прочитать итоговую строку.
            size_read = read(output_fifo, read_string, 5000);
            if (size_read < 0) {
                printf("Child_one can't read string from second fifo\n");
            }
            // Пытаемся открыть обычный файл для записи итогового результата.
            if ((input = open(write_to, O_CREAT | O_WRONLY, 0666)) < 0) {
                printf("Child_one can't open file to write result\n");
            }
            size_write = write(input, read_string, strlen(read_string));
            if (size_write != strlen(read_string)) {
                printf("Child_one can't write result string\n");
            }
            printf("Child_one has written result string\n");
            remove(name_fifo);
        }
    }
}

