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

// Функция поливки цветов.
// Запоминаем значения цветов, которые полил первый садовник, чтобы потом отправить info клиенту через сервер.
void wateringFlowers(char *flowers, int *watered_flowers) {
    int count = 0;
    for (int i = 0; i < 20; ++i) {
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
    char ready_message[39] = "First client is ready for communication";
    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        printf("Socket connection failed for first client\n");
        exit(-1);
    }

    // Установка данных ip и порта.
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[2]);
    servaddr.sin_port = htons(port);

    int len = sizeof(servaddr);
    // Отправляем на сервер сообщение о том, что первый клиент готов к обмену данными.
    sendto(socket_fd, (const char *) ready_message, strlen(ready_message),
           MSG_CONFIRM, (const struct sockaddr *) &servaddr, len);

    char flowers[40];// Сюда помещаются данные о поливки цветов.

    int watered_flowers[21];// Цветы, которые политы первым садовником.

    // В цикле каждые пять дней читаем данные о поливке цветов.
    // Серверу возвращаем информацию о номерах политых цветов.
    for (int i = 0; i < 5; ++i) {
        recvfrom(socket_fd, flowers, sizeof(flowers), MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
        wateringFlowers(flowers, watered_flowers);
        sendto(socket_fd, watered_flowers, sizeof(watered_flowers), MSG_CONFIRM, (const struct sockaddr *) &servaddr, len);
    }
    close(connection_fd);
}
