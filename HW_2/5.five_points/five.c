#include "sys/mman.h"
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

void *handler(int);
char *shared_memory_name; // Имя разделяемой памяти.
sem_t sem_place_parent;
sem_t sem_place_child_first;
sem_t sem_place_child_second;

//Обработчик сигнала нажатия ctrl + c (т.е завершения работы программы).
void *my_handler(int n_sig) {
    // Удаление семафоров из памяти.
    sem_destroy(&sem_place_parent);
    sem_destroy(&sem_place_child_first);
    sem_destroy(&sem_place_child_second);
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
    srand(time(NULL));
    int sem_return;
    shared_memory_name = "memory";
    int shm_fd = shm_open(shared_memory_name, O_CREAT | O_RDWR, 0666);// Файловый дескриптор.
    ftruncate(shm_fd, 4096);                                          // Обрезаем файл до указанной длины.
    int *ptr_memory = (int *) mmap(0, 4096, PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd, 0);
    pid_t child_first, child_second;// Два садовника.
    // Далее создание и инициализация 3 семафоров, которые будут организовывать работу сада.
    if ((sem_return = sem_init(&sem_place_parent, 1, 1)) < 0) {
        printf("Can't create semaphore in parent\n");
        exit(-1);
    }

    if ((sem_return = sem_init(&sem_place_child_first, 1, 0)) < 0) {
        printf("Can't create semaphore in parent\n");
        exit(-1);
    }

    if ((sem_return = sem_init(&sem_place_child_second, 1, 0)) < 0) {
        printf("Can't create semaphore in parent\n");
        exit(-1);
    }
    // Массив цветов. 1 - политы, 0 - не политы
    int num;
    int flowers[40];
    // Изначально говорим, что все политы. Начиная с первого дня, какие-то цветы начинают вянуть.
    for (int i = 0; i < 40; ++i) {
        flowers[i] = 1;
    }
    // Первый садовник.
    child_first = fork();
    // Второй садовник.
    child_second = fork();
    if (child_first == -1) {
        printf("Can't creat child_first process\n");
    } else if (child_first > 0 && child_second > 0) {// Родительский процесс (условно говоря сад).
        // sleep() помогает иллюстрировать настоящую работу сада.
        (void *) signal(SIGINT, my_handler);
        printf("%s", "В саду появилась новая клумба, на ней посажено 40 цветов\n");
        sleep(1);
        printf("%s", "Ищем садовников...\n");
        sleep(2);
        printf("%s", "Ура! Майкл и Джон будут ухаживать за цветами\n");
        // Есть 20 дней полива, в любой момент можно остановить работу сада при помощи ctrl + c.
        // Если не завершить при помощи ctrl + c, то программа завершится сама после обработки 20 дней.
        // В зависимости от num выбираются цветы для поливки.
        for (int i = 0; i < 20; ++i) {
            sem_wait(&sem_place_parent);// Родитель (сад) ждёт своего запуска.
            if (i > 0) {
                // Этот sleep для того, чтобы дни полива друг от друга отделялись, а не выводились в кучу.
                sleep(2);
            }
            printf("\n\nДень: %d\n", i + 1);
            num = 1 + rand() % (10 - 1);
            changeArrayFlowers(flowers, ptr_memory, num);
            sem_post(&sem_place_child_first);// Как только решили, какие цветы надо поливать - запуск первого садовника, он в свою очередь уже разрешит работу для второго.

        }
        // Эти удаления нужны для случая, если пользователь не завершил программу при помощи ctrl + c.
        // Удаление семафоров из памяти.
        sem_wait(&sem_place_parent);
        sem_destroy(&sem_place_parent);
        sem_destroy(&sem_place_child_first);
        sem_destroy(&sem_place_child_second);
        // Удаление разделяемой памяти.
        shm_unlink(shared_memory_name);
        printf("Полив окончен\n");
        exit(0);
    } else if (child_first == 0) {// Первый садовник.
        // Первый садовник поливает цветы с 1 по 20.
        // При этом первый садовник при помощи семафоров регулирует работу второго.
        for (int i = 0; i < 20; ++i) {
            int sem_val;
            sem_getvalue(&sem_place_child_first, &sem_val);
            printf("%d\n", sem_val);
            sem_wait(&sem_place_child_first); // Первый садовник ждёт запуска.
            sem_post(&sem_place_child_second);// Запуск второго садовника.
            for (int j = 0; j < 20; ++j) {
                // Если 0, то цветок не полит, значит мы его поливаем и говорим, что теперь он равен 1.
                if (ptr_memory[j] == 0) {
                    printf("Майкл поливает цветок %d\n", j + 1);
                    ptr_memory[j] = 1;
                }
            }
            sem_post(&sem_place_parent);// Запуск родителя (т.е работы сада).
        }
    } else if (child_second == 0) {// Второй садовник.
        // Первый садовник при помощи sem_post пробуждает второго.
        // Они вместе поливают цветы, первый с 1 по 19, второй с 20 по 40
        for (int i = 0; i < 20; ++i) {
            sem_wait(&sem_place_child_second);// Второй садовник ждёт запуска.
            for (int j = 20; j < 40; ++j) {
                // Если 0, то цветок не полит, значит мы его поливаем и говорим, что теперь он равен 1.
                if (ptr_memory[j] == 0) {
                    printf("Джон поливает цветок %d\n", j + 1);
                    ptr_memory[j] = 1;
                }
            }
        }
    }
}
