#include "sys/mman.h"
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int *ptr_memory;
    int mem_id;                          // IPC дескриптор для области разделяемой памяти.
    char file_name[] = "memory_systemV"; // Имя файла для генерации ключа.
    key_t key;                           // IPC ключ.
    if ((key = ftok(file_name, 0)) < 0) {// Генерирую IPC ключ.
        printf("Can\'t generate key\n");
        exit(-1);
    }
    if ((mem_id = shmget(key, 40 * sizeof(int), 0666)) < 0) {
        printf("Проблемы с созданием разделяемой памяти\n");
        exit(-1);
    }
    // Отображаем разделяемую память в указатель на int.
    if ((ptr_memory = (int *) shmat(mem_id, NULL, 0)) == (int *) (-1)) {
        printf("Can't attach shared memory\n");
        exit(-1);
    }

    // Семафор для 2 ребёнка (2-ого садовника).
    char sem_childS_file[] = "child_second";
    key_t key_childS;
    key_childS = ftok(sem_childS_file, 0);
    int sem_childS_id = semget(key_childS, 1, 0666);
    // Структура для ожидания, пока значение семафора не достигнет 1.
    struct sembuf wait;
    wait.sem_op = -1;
    wait.sem_num = 0;
    wait.sem_flg = 0;

    // Первый садовник пробуждает второго.
    for (int i = 0; i < 20; ++i) {
        semop(sem_childS_id, &wait, 1);// Второй садовник ждёт запуска.
        for (int j = 20; j < 40; ++j) {
            // Если 0, то цветок не полит, значит мы его поливаем и говорим, что теперь он равен 1.
            if (ptr_memory[j] == 0) {
                printf("Джон поливает цветок %d\n", j + 1);
                ptr_memory[j] = 1;
            }
            if (j == 39) {
                printf("\n\n");
            }
        }
    }
}
