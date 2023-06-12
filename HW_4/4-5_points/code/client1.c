#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define sa struct sockaddr

// Функция, меняющая номер дня полива.
// Далее эта информация отправляется серверу.
void changeMessage(char *message, int day) {
    switch (day) {
        case 1:
            message[0] = '1';
            break;
        case 2:
            message[0] = '2';
            break;
        case 3:
            message[0] = '3';
            break;
        case 4:
            message[0] = '4';
            break;
        case 5:
            message[0] = '5';
            break;
        default:
            break;
    }
}

// Функция поливки цветов. Если 0, то цветок нужно полить и вывести соотв. сообщение.
// Первый садовник может поливать цветы только с 1 по 20.
void wateringFlowers(char *flowers) {
    for (int i = 0; i < 20; ++i) {
        if (flowers[i] == '0') {
            printf("First gardener is watering flower number %d\n", i + 1);
        }
    }
    printf("\n");
}
int main(int argc, char *argv[]) {
    int socket_fd;
    int port = atoi(argv[1]);
    struct sockaddr_in servaddr, client;
    char ready_message[39] = "First client is ready for communication";
    // Создание UDP сокета
    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        printf("Socket creation failed for first client\n");
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
    // Сообщение, которое отправляется серверу. Первый элемент зависит от номера дня.
    char message[41] = "  watering day is over for first gardener";
    // В цикле в каждый из 5 дней получаем данные от сервера о поливке цветов.
    // Далее поливаем эти цветы и в конце поливки отправляем информацию о том, что полив в очередной день окончен.
    for (int i = 0; i < 5; ++i) {
        recvfrom(socket_fd, flowers, sizeof(flowers), MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
        wateringFlowers(flowers);
        changeMessage(message, i + 1);
        sendto(socket_fd, message, strlen(message), MSG_CONFIRM, (const struct sockaddr *) &servaddr, len);
    }
}
