#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


int main(int argc, char *argv[]) {
    char *read_or_write = argv[1];// флаг чтения или записи
    char fifo_name[9] = "work.fifo";// имя канала для обмена
    int input, output, input_fifo, output_fifo, size_read, size_write;
    char *write_string = (char *) calloc(5000, sizeof(char));// Для записи в канал.
    char *read_string = (char *) calloc(5000, sizeof(char)); // Для чтения из канала.
    // При запуске пришло "r" - значит читаем строку из обычного файла.
    if (strcmp(read_or_write, "r") == 0) {
        if ((input = open(argv[2], O_RDONLY)) < 0) {
            printf("First process can't open file\n");
            exit(-1);
        }
        // Пытаемся открыть файл для чтения строки.
        size_read = read(input, read_string, 5000);
        if (size_read < 0) {
            printf("First process can't read string from file\n");
            exit(-1);
        }
        close(input);
        // Пытаемся создать канал для двух независимых процессов.
        if (mknod(fifo_name, S_IFIFO | 0666, 0) < 0) {
            printf("First process can't create FIFO\n");
            exit(-1);
        }
        // Пытаемся открыть fifo для передачи данных.
        if ((input_fifo = open(fifo_name, O_WRONLY)) < 0) {
            printf("First process can't open fifo for writing\n");
            exit(-1);
        }
        size_write = write(input_fifo, read_string, strlen(read_string));
        if (size_write != strlen(read_string)) {
            printf("First process can't write string to fifo\n");
            exit(-1);
        }
        printf("First process send info to second process using pipe\n");
        close(input_fifo);
    }
    // В данном случае эта программа будет уже запущена второй раз.
    // На этот момент второй процесс уже успеет обработать строку и передать назад.
    else if (strcmp(argv[1], "w") == 0){
        // Открываем канал на чтение итогового результа, полученного от второго процесса.
        if((output_fifo = open(fifo_name, O_RDONLY))< 0){
            printf("First process can't open fifo for reading\n");
            exit(-1);
        }
        // Пытаемся прочитать из канала.
        size_read = read(output_fifo, read_string, 5000);
        if (size_read < 0){
            printf("First process can't read string using fifo\n");
            exit(-1);
        }
        close(output_fifo);
        // Пытаемся открыть файл для записи итогового результата.
        if ((output = open(argv[2], O_CREAT | O_WRONLY, 0666))< 0){
            printf("First process can't open file to write result\n");
            exit(-1);
        }
        // Пытаемся записать итоговый результат в файл.
        size_write = write(output, read_string, strlen(read_string));
        if (size_write != strlen(read_string)){
            printf("First process can't write result to file\n");
            exit(-1);
        }
        close(output);
        printf("First process has written result to file\n");
        remove(fifo_name);
    }
}
