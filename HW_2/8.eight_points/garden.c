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
int *ptr_memory;                     // Указатель на разделяемую память.
int sem_parent_id; // Семафор для контроля родительского процесса (сада).
int sem_childF_id; // Семафор для контроля первого дочернего процесса (1 садовник).
int sem_childS_id; // Семафор для контроля второго дочернего процесса (2 садовник).
//Обработчик сигнала нажатия ctrl + c (т.е завершения работы программы).
void *my_handler(int n_sig) {
    // Удаление разделяемой памяти.
    shmdt(ptr_memory);
    // Удаление 3 семафоров.
    semctl(sem_parent_id, 0, IPC_RMID, 0);
    semctl(sem_childF_id, 0, IPC_RMID, 0);
    semctl(sem_childS_id, 0, IPC_RMID, 0);
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
    srand(time(NULL));
    int mem_id;                          // IPC дескриптор для области разделяемой памяти.
    char file_name[] = "memory_systemV"; // Имя файла для генерации ключа.
    key_t key;                           // IPC ключ.
    if ((key = ftok(file_name, 0)) < 0) {// Генерирую IPC ключ.
        printf("Can\'t generate key\n");
        exit(-1);
    }
    if ((mem_id = shmget(key, 40 * sizeof(int), 0666 | IPC_CREAT)) < 0) {
        printf("Проблемы с созданием разделяемой памяти\n");
        exit(-1);
    }
    // Отображаем разделяемую память в указатель на int.
    if ((ptr_memory = (int *) shmat(mem_id, NULL, 0)) == (int *) (-1)) {
        printf("Can't attach shared memory\n");
        exit(-1);
    }
    pid_t child_first, child_second;// Два садовника.
    // Далее создание и инициализация 3 семафоров, которые будут организовывать работу сада.
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
    key_parent = ftok(sem_parent_file,0);
    sem_parent_id = semget(key_parent, 1, 0666 | IPC_CREAT);
    semctl(sem_parent_id, 0, SETVAL, 1);

    // Семафор для 1 ребёнка (1-ого садовника).
    char sem_childF_file[] = "child_first";
    key_t key_childF;
    key_childF = ftok(sem_childF_file,0);
    sem_childF_id = semget(key_childF, 1, 0666 | IPC_CREAT);
    semctl(sem_childF_id, 0, SETVAL, 0);

    // Семафор для 2 ребёнка (2-ого садовника).
    char sem_childS_file[] = "child_second";
    key_t key_childS;
    key_childS = ftok(sem_childS_file,0);
    sem_childS_id = semget(key_childS, 1, 0666 | IPC_CREAT);
    semctl(sem_childS_id, 0, SETVAL, 0);
    printf("%s", "В саду появилась новая клумба, на ней посажено 40 цветов\n");
    sleep(1);
    printf("%s", "Ищем садовников...\n");
    sleep(2);
    printf("%s", "Ура! Майкл и Джон будут ухаживать за цветами\n");
    // Есть 20 дней полива, в любой момент можно остановить работу сада при помощи ctrl + c.
    // Если не завершить при помощи ctrl + c, то программа завершится сама после обработки 20 дней.
    // В зависимости от num выбираются цветы для поливки.
    // Массив цветов. 1 - политы, 0 - не политы
    int num;
    int flowers[40];
    // Изначально говорим, что все политы. Начиная с первого дня, какие-то цветы начинают вянуть.
    for (int i = 0; i < 40; ++i) {
        flowers[i] = 1;
    }
    for (int i = 0; i < 20; ++i) {
        semop(sem_parent_id, &wait, 1);// Родитель (сад) ждёт своего запуска.
        if (i > 0) {
            // Этот sleep для того, чтобы дни полива друг от друга отделялись, а не выводились в кучу.
            sleep(2);
        }
        printf("\n\nДень: %d\n", i + 1);
        num = 1 + rand() % (10 - 1);
        changeArrayFlowers(flowers, ptr_memory, num);
        semop(sem_childF_id, &plus_one, 1);// Как только решили, какие цветы надо поливать - запуск первого садовника, он в свою очередь уже разрешит работу для второго.
    }
}
