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

// Функция выводит сообщение, полученное от очередного клиента.
void printMessage(char *message, int len) {
    for (int i = 0; i < len; ++i) {
        printf("%c", message[i]);
    }
    printf("\n");
}

// Функция выбирает, какие цветы нужно поливать в очередной день.
// Выбор зависит от деления на число num.
void chooseFlowersForWatering(char *flowers, int num) {
    for (int i = 0; i < 40; ++i) {
        if (i % num == 0) {
            flowers[i] = '0';
        }
    }
}
int main(int argc, char *argv[]) {
    srand(time(NULL));
    int socket_fd, num;
    int port = atoi(argv[1]);
    struct sockaddr_in servaddr, first_client, second_client;
    char flowers[40];// Клумба из 40 цветов.
                     // Изначально все цветы будут политы ('1' - полит цветок, '0' - не полит).
    for (int i = 0; i < 40; ++i) {
        flowers[i] = '1';
    }
    // Попытка создания сокета.
    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        printf("Failed to create socket on server...\n");
        exit(0);
    }
    // Установка данных для соединения.
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    // Связываем сокет с данными из структуры.
    if ((bind(socket_fd, (sa *) &servaddr, sizeof(servaddr))) != 0) {
        printf("Socket bind failed...\n");
        exit(0);
    }
    int len = sizeof(servaddr);
    printf("Server is waiting for clients...\n");
    char ready_message[45];
    // Получаем сообщение от первого клиента о готовности обмена данными.
    recvfrom(socket_fd, ready_message, sizeof(ready_message), MSG_CONFIRM, (const struct sockaddr *) &first_client, &len);
    printMessage(ready_message, sizeof(ready_message));
    // Получаем сообщение от первого клиента о готовности обмена данными.
    recvfrom(socket_fd, ready_message, sizeof(ready_message), MSG_CONFIRM, (const struct sockaddr *) &second_client, &len);
    printMessage(ready_message, sizeof(ready_message));
    char message_first[41]; // Сообщения от первого клиента.
    char message_second[42];// Сообщения от второго клиента.
    // В цикле на протяжении 5 дней организуем работу полива.
    // Для начала рандомно определяем цветы, которые нужно будет полить.
    // Далее отправляем данные о поливке первому клиенту (садовнику), после второму клиенту (садовнику).
    // Т.е сервер отправляет сообщение с данными о поливке, а получает в ответ результат данного полива.
    for (int i = 0; i < 5; ++i) {
        num = 1 + rand() % (10 - 1);
        chooseFlowersForWatering(flowers, num);
        // Общение с первым садовником.
        sendto(socket_fd, flowers, sizeof(flowers), MSG_CONFIRM, (const struct sockaddr *) &first_client, len);
        recvfrom(socket_fd, message_first, sizeof(message_first), MSG_WAITALL, (struct sockaddr *) &first_client, &len);
        printMessage(message_first, sizeof(message_first));
        sleep(2);
        // Общение со вторым садовником.
        sendto(socket_fd, flowers, sizeof(flowers), MSG_CONFIRM, (const struct sockaddr *) &second_client, len);
        recvfrom(socket_fd, message_second, sizeof(message_second), MSG_WAITALL, (struct sockaddr *) &second_client, &len);
        printMessage(message_second, sizeof(message_second));
    }
}

