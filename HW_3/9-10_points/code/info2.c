#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define sa struct sockaddr

// Вывод информации о поливке цветов.
// Данная функция вызывается отдельно для каждого садовника.
// watered_flowers хранит номера политых цветов.
void printMessage(int* watered_flowers, int gardener){
    for (int i = 1; i < watered_flowers[0]; ++i) {
        if(gardener == 1){
            printf("First gardener is watering flower number %d\n", watered_flowers[i]);
        }
        else{
            printf("Second gardener is watering flower number %d\n", watered_flowers[i]);
        }
    }
}
int main(int argc, char *argv[]) {
    int socket_fd;
    int connection_fd;
    int port = atoi(argv[1]);
    struct sockaddr_in servaddr, client;
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("Socket connection failed\n");
    }

    // Установка данных ip и порта.
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[2]);
    servaddr.sin_port = htons(port);

    // Попытка соединиться с сервером.
    if (connect(socket_fd, (sa *) &servaddr, sizeof(servaddr)) != 0) {
        printf("Connection with the server failed...\n");
        exit(0);
    }
    int start_day[1];
    read(socket_fd, start_day, sizeof(start_day));
    int watered_flowers[21]; // Номера политых цветов садовниками.
    // В цикле каждые пять дней получаем от сервера информацию о цветах, политых садовниками.
    for (int i = 0; i < 5; ++i) {
        printf("Watering day number %d\n", i + 1 + start_day[0]);
        // Информация о 1 садовнике.
        read(socket_fd, watered_flowers, sizeof(watered_flowers));
        // Если первое число в watered_flowers равно -1, то значит, что info клиент должен завершить отображение информации.
        // -1 была помещена в массив сервером, когда ему пришёл сигнал ctrl+c, где был выбран соотв. info клиент для отключения.
        if(watered_flowers[0]  == -1){
            printf("Disconnection with server...\n");
            exit(0);
        }
        printMessage(watered_flowers, 1);
        printf("First gardener has finished\n\n");
        // Информация о 2 садовнике.
        read(socket_fd, watered_flowers, sizeof(watered_flowers));
        // Если первое число в watered_flowers равно -1, то значит, что info клиент должен завершить отображение информации.
        // -1 была помещена в массив сервером, когда ему пришёл сигнал ctrl+c, где был выбран соотв. info клиент для отключения.
        if(watered_flowers[0]  == -1){
            printf("Disconnection with server...\n");
            exit(0);
        }
        printMessage(watered_flowers, 2);
        printf("Second gardener has finished\n\n");
    }
    close(connection_fd);
}
