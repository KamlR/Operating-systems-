#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
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
    char *read_from = argv[1];                               // Файл, из которого будет читаться строка.
    char *write_to = argv[2];                                // Файл, в который будет записываться итоговый результат.
    char *write_string = (char *) calloc(5000, sizeof(char));// Для записи в pipe.
    char *read_string = (char *) calloc(5000, sizeof(char)); // Для чтения из pipe.
    int first_fd[2], second_fd[2], input, output, size_read, size_write;
    pid_t child;
    // Пытаюсь создать pipe для связи вида: родитель - ребёнок.
    // Поясню. Через данный pipe родитель передаст информацию ребёнку, а тот считает её.
    if (pipe(first_fd) < 0) {
        printf("Can't create pipe parent - child\n");
        exit(-1);
    }
    // Создание второго pipe.
    // Через него ребёнок передаёт родителю информацию о кол-ве гласных и согласных.
    if (pipe(second_fd) < 0) {
        printf("Can't create pipe child - parent\n");
    }
    // Создаём дочерний процесс.
    child = fork();
    if (child < 0) {
        printf("Can't create child process\n");
        exit(-1);
    }
    // Заходим в родительский процесс.
    // Он сейчас будет писать данные в первый pipe.
    else if (child > 0) {
        input = open(read_from, O_RDONLY);          // Открываем файл для чтения строки.
        size_read = read(input, write_string, 5000);// Читаем строку в write_string.
        if (size_read < 0) {
            printf("Parent can't read string from file\n");
            exit(-1);
        }
        close(first_fd[0]);
        // Передаём инфомацию в pipe через входной поток данных.
        size_write = write(first_fd[1], write_string, strlen(write_string));
        if (size_write != strlen(write_string)) {
            printf("Parent can't write all string to pipe\n");
            exit(-1);
        }
        close(first_fd[1]);
        printf("Parent passed data to child using pipe\n");
    }
    // Заходим к ребёнку.
    // Он сейчас будет читать данные из первого pipe.
    else if (child == 0) {
        close(first_fd[1]);
        // Из pipe читаем строку для обработки.
        size_read = read(first_fd[0], read_string, 5000);
        if (size_read < 0) {
            printf("Child can't read a string using pipe\n");
            exit(-1);
        }
        close(first_fd[0]);
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
        close(second_fd[0]);
        // Уже через второй pipe мы передаём данные назад первому процессу.
        size_write = write(second_fd[1], vowels, strlen(vowels));
        if (size_write != strlen(vowels)) {
            printf("Child can't write all string to second pipe\n");
            exit(-1);
        }
        printf("Child has sent data to parent using pipe\n");
        close(second_fd[1]);
    }

    // Если мы опять оказались в родительском процессе.
    // Он уже из второго pipe читает данные от ребёнка и пишет их в обычный файл.
    // Почему не возникнет ситуации, когда родитель уже читает, а ребёнок ещё не записал?
    // Я делаю sleep(3). За эти три секунды процесс ребёнок успевает записать данные. Т.е родитель заходит, ждёт 3 секунды, а потом уже читает результат.
    if (child > 0) {
        close(second_fd[1]);
        sleep(3);
        // Читаем уже из второго pipe обработанный результат.
        size_read = read(second_fd[0], read_string, 5000);
        if (size_read < 0) {
            printf("Parent can't read info from child\n");
            exit(-1);
        }
        close(second_fd[0]);
        // Пытаемся открыть файл для записи результата.
        if ((output = open(write_to, O_CREAT | O_WRONLY, 0666)) < 0) {
            printf("Parent can't open file to write result\n");
            exit(-1);
        }
        // Пишем, сколько гласных и согласных в исходной строке.
        size_write = write(output, read_string, strlen(read_string));
        if (size_write != strlen(read_string)) {
            printf("Parent can't write result to file\n");
            exit(-1);
        }
        close(output);
        printf("Parent successfully wrote info to file\n");
    }
}

