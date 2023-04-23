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

    // Структура для увеличения семафора на 1.
    struct sembuf plus_one;
    plus_one.sem_op = 1;
    plus_one.sem_num = 0;
    plus_one.sem_flg = 0;

    // Структура для ожидания, пока значение семафора не достигнет 1.
    struct sembuf wait;
    wait.sem_op = -1;
    wait.sem_num = 0;
    wait.sem_flg = 0;

    // Семафор для родителя (сада).
    char sem_parent_file[] = "parent";
    key_t key_parent;
    key_parent = ftok(sem_parent_file, 0);
    int sem_parent_id = semget(key_parent, 1, 0666);

    // Семафор для 1 ребёнка (1-ого садовника).
    char sem_childF_file[] = "child_first";
    key_t key_childF;
    key_childF = ftok(sem_childF_file, 0);
    int sem_childF_id = semget(key_childF, 1, 0666);

    // Семафор для 2 ребёнка (2-ого садовника).
    char sem_childS_file[] = "child_second";
    key_t key_childS;
    key_childS = ftok(sem_childS_file, 0);
    int sem_childS_id = semget(key_childS, 1, 0666);

    for (int i = 0; i < 20; ++i) {
        semop(sem_childF_id, &wait, 1);    // Первый садовник ждёт запуска.
        semop(sem_childS_id, &plus_one, 1);// Запуск второго садовника.
        for (int j = 0; j < 20; ++j) {
            // Если 0, то цветок не полит, значит мы его поливаем и говорим, что теперь он равен 1.
            if (ptr_memory[j] == 0) {
                printf("Майкл поливает цветок %d\n", j + 1);
                ptr_memory[j] = 1;
            }
            if (j == 19) {
                printf("\n\n");
            }
        }
        semop(sem_parent_id, &plus_one, 1);// Запуск родителя (т.е работы сада).
    }
}
