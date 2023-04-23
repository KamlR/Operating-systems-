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
    char *shared_memory_name = "shared_memory";             // Имя разделяемой памяти.
    int shm_fd = shm_open(shared_memory_name, O_RDWR, 0666);// Файловый дескриптор.
    int *ptr_memory = (int *) mmap(0, 4096, PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd, 0);

    // Разделяемая память для семафора.
    int mem_id;                          // IPC дескриптор для области разделяемой памяти.
    char file_name[] = "sem_memory";     // Имя файла для генерации ключа.
    key_t key;                           // IPC ключ.
    if ((key = ftok(file_name, 0)) < 0) {// Генерирую IPC ключ.
        printf("Can\'t generate key\n");
        exit(-1);
    }
    if ((mem_id = shmget(key, 2 * sizeof(sem_t *), 0666)) < 0) {
        printf("Проблемы с созданием разделяемой памяти\n");
        exit(-1);
    }
    // Отображаем разделяемую память в указатель на int.
    sem_t **sem_place;
    if ((sem_place = (sem_t **) shmat(mem_id, NULL, 0)) == (sem_t **) (-1)) {
        printf("Can't attach shared memory\n");
        exit(-1);
    }
    // Первый садовник поливает цветы с 1 по 20.
    // При этом первый садовник при помощи семафоров регулирует работу второго.
    for (int i = 0; i < 20; ++i) {
        sem_wait(*(sem_place + 1));// Первый садовник ждёт запуска.
        sem_post(*(sem_place + 2)); // Запускаем второго садовника.
        for (int j = 0; j < 20; ++j) {
            // Если 0, то цветок не полит, значит мы его поливаем и говорим, что теперь он равен 1.
            if (ptr_memory[j] == 0) {
                printf("Майкл поливает цветок %d\n", j + 1);
                ptr_memory[j] = 1;
            }
        }
        sem_post(*sem_place);// Запуск родителя (т.е работы сада).
    }
}
