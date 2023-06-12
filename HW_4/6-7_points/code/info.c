#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
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
    char ready_message[39] = "Info client is ready for communication";
    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        printf("Socket connection failed for info client\n");
    }

    // Установка данных ip и порта.
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[2]);
    servaddr.sin_port = htons(port);

    int len = sizeof(servaddr);
    // Отправляем на сервер сообщение о том, что инфо клиент готов к обмену данными.
    sendto(socket_fd, (const char *) ready_message, strlen(ready_message),
           MSG_CONFIRM, (const struct sockaddr *) &servaddr, len);

    int watered_flowers[21]; // Номера политых цветов садовниками.
    // В цикле каждые пять дней получаем от сервера информацию о цветах, политых садовниками.
    for (int i = 0; i < 5; ++i) {
        printf("Watering day number %d\n", i + 1);
        // Информация о 1 садовнике.
        recvfrom(socket_fd, watered_flowers, sizeof(watered_flowers), MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
        printMessage(watered_flowers, 1);
        printf("First gardener has finished\n\n");
        // Информация о 2 садовнике.
        recvfrom(socket_fd, watered_flowers, sizeof(watered_flowers), MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
        printMessage(watered_flowers, 2);
        printf("Second gardener has finished\n\n");
    }
}
