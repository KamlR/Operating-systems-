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
    char *write_string = (char *) calloc(5000, sizeof(char));// Для записи в канал.
    char *read_string = (char *) calloc(5000, sizeof(char)); // Для чтения из канала.
    char *fifo_name = "work.fifo";
    int input_fifo, output_fifo, size_write, size_read;
    // Пытаемся открыть канал для чтения.
    if ((output_fifo = open(fifo_name, O_RDONLY)) < 0) {
        printf("Second process can't open fifo for reading\n");
        exit(-1);
    }
    // Пытаемся прочитать строку из канала, которую передал 1 процесс.
    size_read = read(output_fifo, read_string, 5000);
    if (size_read < 0) {
        printf("Second process can't read from fifo\n");
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
    // Теперь результат о количестве гласных и согласных передаём 1 процессу через канал.
    // Пытаемся открыть канал для записи.
    if ((input_fifo = open(fifo_name, O_WRONLY)) < 0) {
        printf("Second process can't open fifo for writing\n");
        exit(-1);
    }
    // Пытаемся записать обработанную информацию в fifo.
    size_write = write(input_fifo, vowels, strlen(vowels));
    if (size_write != strlen(vowels)) {
        printf("Second process can't write string to fifo\n");
        exit(-1);
    }
    close(input_fifo);
    printf("Second process has written result string to fifo\n");
}
