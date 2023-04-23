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

void *handler(int);
char * sem_name_parent;
char* sem_name_child_first;
char* sem_name_child_second;
char * shared_memory_name;
//Обработчик сигнала нажатия ctrl + c (т.е завершения работы программы).
void *my_handler(int n_sig) {
    // Удаление семафоров из памяти.
    sem_unlink(sem_name_parent);
    sem_unlink(sem_name_child_first);
    sem_unlink(sem_name_child_second);
    // Удаление разделяемой памяти.
    shm_unlink(shared_memory_name);
    printf("\n\nНаступила зима, сад временно не работает\n");
    exit(0);
}
// Функция для выбора цветов, которые нужно поливать на текущий день.
// Здесь я сразу записываю данные в разделяемую память.
void changeArrayFlowers(int flowers[], int *ptr_memory, int num) {
    for (int i = 0; i < 40; ++i) {
        if (i % num == 0) {
            flowers[i] = 0;
        }
        ptr_memory[i] = flowers[i];
    }
}
int main(int argc, char *argv[]) {
    (void *) signal(SIGINT, my_handler);
    shared_memory_name = "shared_memory";                       // Имя разделяемой памяти.
    int shm_fd = shm_open(shared_memory_name, O_CREAT | O_RDWR, 0666);// Файловый дескриптор.
    ftruncate(shm_fd, 4096);                                          // Обрезаем файл до указанной длины.
    int *ptr_memory = (int *) mmap(0, 4096, PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd, 0);

    // Далее создание и инициализация 3 семафоров, которые будут организовывать работу сада.
    sem_name_parent = "control_flowers_parent";
    sem_t *sem_place_parent;
    if ((sem_place_parent = sem_open(sem_name_parent, O_CREAT | O_RDWR, 066, 1)) == 0) {
        printf("Can't create semaphore for parent\n");
        exit(-1);
    }
    sem_name_child_first = "control_flowers_child_first";
    sem_t *sem_place_child_first;
    if ((sem_place_child_first = sem_open(sem_name_child_first, O_CREAT | O_RDWR, 066, 0)) == 0) {
        printf("Can't create semaphore for first child\n");
        exit(-1);
    }
    sem_name_child_second = "control_flowers_child_second";
    sem_t *sem_place_child_second;
    if ((sem_place_child_second = sem_open(sem_name_child_second, O_CREAT | O_RDWR, 066, 0)) == 0) {
        printf("Can't create semaphore for second child\n");
        exit(-1);
    }
    // Разделяемая память для семафора. Хочу так передавать информацию о семафоре другим процессам.
    int mem_id;                          // IPC дескриптор для области разделяемой памяти.
    char file_name[] = "sem_memory";     // Имя файла для генерации ключа.
    key_t key;                           // IPC ключ.
    if ((key = ftok(file_name, 0)) < 0) {// Генерирую IPC ключ.
        printf("Can\'t generate key\n");
        exit(-1);
    }
    if ((mem_id = shmget(key, 3 * sizeof(sem_t *), 0666 | IPC_CREAT)) < 0) {
        printf("Проблемы с созданием разделяемой памяти\n");
        exit(-1);
    }
    sem_t **sem_place;
    if ((sem_place = (sem_t **) shmat(mem_id, NULL, 0)) == (sem_t **) (-1)) {
        printf("Can't attach shared memory\n");
        exit(-1);
    }
    *sem_place = sem_place_parent;
    *(sem_place + 1) = sem_place_child_first;
    *(sem_place + 2) = sem_place_child_second;
    // Массив цветов. 1 - политы, 0 - не политы
    int num;
    int flowers[40];
    // Изначально говорим, что все политы. Начиная с первого дня, какие-то цветы начинают вянуть.
    for (int i = 0; i < 40; ++i) {
        flowers[i] = 1;
    }
    printf("%s", "В саду появилась новая клумба, на ней посажено 40 цветов\n");
    sleep(1);
    printf("%s", "Ищем садовников...\n");
    sleep(2);
    printf("%s", "Ура! Майкл и Джон будут ухаживать за цветами\n");

    // Есть 20 дней полива, в любой момент можно остановить работу сада при помощи ctrl + c.
    // Если не завершить при помощи ctrl + c, то программа завершится сама после обработки 20 дней.
    // В зависимости от num выбираются цветы для поливки.
    for (int i = 0; i < 20; ++i) {
        sem_wait(sem_place_parent);// Родитель (сад) ждёт своего запуска.
        if (i > 0) {
            // Этот sleep для того, чтобы дни полива друг от друга отделялись, а не выводились в кучу.
            sleep(2);
        }
        printf("\n\nДень: %d\n", i + 1);
        num = 1 + rand() % (10 - 1);
        changeArrayFlowers(flowers, ptr_memory, num);
        sem_post(sem_place_child_first);// Как только решили, какие цветы надо поливать - запуск первого садовника, он в свою очередь уже разрешит работу для второго.
    }
    // Эти удаления нужны для случая, если пользователь не завершил программу при помощи ctrl + c.
    sem_wait(sem_place_parent);
    // Удаление семафоров из памяти.
    sem_unlink(sem_name_parent);
    sem_unlink(sem_name_child_first);
    sem_unlink(sem_name_child_second);
    // Удаление разделяемой памяти.
    shm_unlink(shared_memory_name);
    printf("Полив окончен\n");
    exit(0);
    return 0;
}
