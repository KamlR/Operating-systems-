#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define sa struct sockaddr

// Функция поливки цветов.
// Запоминаем значения цветов, которые полил второй садовник, чтобы потом отправить info клиенту через сервер.
void wateringFlowers(char *flowers, int *watered_flowers) {
    int count = 0;
    for (int i = 20; i < 40; ++i) {
        if (flowers[i] == '0') {
            watered_flowers[count + 1] = i + 1;
            count += 1;
        }
    }
    watered_flowers[0] = count;
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
    char flowers[40]; // Сюда помещаются данные о поливки цветов.

    int watered_flowers[21]; // Цветы, которые политы первым садовником.

    // В цикле каждые пять дней читаем данные о поливке цветов.
    // Серверу возвращаем информацию о номерах политых цветов.
    for (int i = 0; i < 5; ++i) {
        read(socket_fd, flowers, sizeof(flowers));
        wateringFlowers(flowers, watered_flowers);
        write(socket_fd, watered_flowers, sizeof(watered_flowers));
    }
    close(connection_fd);
}
