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
    char *write_string = (char *) calloc(5000, sizeof(char));// Для записи в канал.
    char *read_string = (char *) calloc(5000, sizeof(char)); // Для чтения из канала.
    int input_fifo, output_fifo, input, output, size_read, size_write;
    pid_t child, grandson;
    char name_fifo[9] = "work.fifo";// Файл для создания fifo.
    (void) umask(0);
    // Создаём fifo в текущей директории для parent и child.
    if (mknod(name_fifo, S_IFIFO | 0666, 0) < 0) {
        printf("Can't create FIFO\n");
        exit(-1);
    }
    child = fork();// Создаю дочерний процесс.
    if (child < 0) {
        printf("Can't creat child\n");
        exit(-1);
    }
    // Если больше 0, то мы оказались в процессе родителе.
    else if (child > 0) {
        // Открываем файл для чтения строки для дальнейшей отработки.
        if ((input = open(read_from, O_RDONLY)) < 0) {
            printf("Parent can't open file to read string for work");
        }
        size_read = read(input, write_string, 5000);// Читаем строку в write_string.
        if (size_read < 0) {
            printf("Parent can't read string from file\n");
            exit(-1);
        }
        // Пытаемся открыть fifo для передачи информации процессу child
        if ((input_fifo = open(name_fifo, O_WRONLY)) < 0) {
            printf("Parent can't open FIFO for writing\n");
            exit(-1);
        }
        // Записываю, считанную строку в fifo.
        size_write = write(input_fifo, write_string, strlen(write_string));
        if (size_write != strlen(write_string)) {
            printf("Parent can't write to fifo\n");
            exit(-1);
        }
        // Закрываю входной поток данных.
        close(input_fifo);
        printf("Parent successfully wrote string using fifo\n");
    }
    // Теперь мы в процессе child, он читает из fifo.
    else {
        // Пытаемся открыть fifo для чтения информации от процесса parent.
        if ((output_fifo = open(name_fifo, O_RDONLY)) < 0) {
            printf("Child can't open fifo for reading");
            exit(-1);
        }
        size_read = read(output_fifo, read_string, 5000);
        if (size_read < 0) {
            printf("Child can't read from fifo");
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
        // Новый fifo для child и grandson.
        char *new_fifo_name = "new.fifo";
        if (mknod(new_fifo_name, S_IFIFO | 0666, 0) < 0) {
            printf("Can't create FIFO\n");
            exit(-1);
        }
        
        grandson = fork();// Создали дочерний процесс от дочернего.
        if (grandson < 0) {
            printf("Can't creat granson process");
        } else if (grandson > 0) {// В процессе child.
            (void) umask(0);
            // Пытаемся открыть fifo для записи информации о кол-ве гласных и согласных granson.
            if ((input_fifo = open(new_fifo_name, O_WRONLY)) < 0) {
                printf("Child can't open FIFO for writing\n");
                exit(-1);
            }
            // Пишем про гласные и согласные.
            size_write = write(input_fifo, vowels, strlen(vowels));
            if (size_write != strlen(vowels)) {
                printf("Child can't pass info to grandson using fifo\n");
                exit(-1);
            }
            printf("Child passed info to grandson using fifo\n");
            // Закрыли поток входной.
            close(input_fifo);
        }
        // Попали в процесс, порождённый child
        else {
            // Открываем fifo для чтения в grandson.
            if ((output_fifo = open(new_fifo_name, O_RDONLY)) < 0) {
                printf("Granson can't open fifo for reading\n");
                exit(-1);
            }
            // Пытаемся прочитать из fifo в строку read_string.
            size_read = read(output_fifo, read_string, 5000);
            if (size_read < 0) {
                printf("Grandson can't read from  fifo\n");
                exit(-1);
            }
            // Закрыли выходной поток.
            close(output_fifo);
            // Теперь итоговый результат о кол-ве гласных и согласных пишем в обычный файл.
            if ((output = open(write_to, O_CREAT | O_WRONLY, 0666)) < 0) {
                printf("Granson can't open file to write result\n");
            }
            // Записали итоговый результат в обычный файл.
            size_write = write(output, read_string, strlen(read_string));
            if (size_write != strlen(read_string)) {
                printf("Grandson can't write result to file");
            }
            printf("Grandson successfully has written result");
            close(output);
            remove(new_fifo_name);
        }
    }
    remove(name_fifo);
}

