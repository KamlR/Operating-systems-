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
    int connection_fd_first, struct_len_first, connection_fd_second, struct_len_second;
    int port = atoi(argv[1]);
    struct sockaddr_in servaddr, first_client, second_client;
    char flowers[40];// Клумба из 40 цветов.
     // Изначально все цветы будут политы ('1' - полит цветок, '0' - не полит).
    for (int i = 0; i < 40; ++i) {
        flowers[i] = '1';
    }
    // Попытка создания сокета.
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
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

    // Говорим, что сервер готов устанавливать связь с клиентами.
    // Также устанавливаем, что только два клиента могут подключаться к серверу (два садовника).
    if ((listen(socket_fd, 2)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }
    printf("Server is waiting for clients...\n");
    // Установка входящего соединения с первым клиентом.
    if ((connection_fd_first = accept(socket_fd, (sa *) &first_client, &struct_len_first)) < 0) {
        printf("Failed to accept first client\n");
    }
    printf("Accept first client\n");

    // Установка входящего соединения со вторым клиентом.
    if ((connection_fd_second = accept(socket_fd, (sa *) &second_client, &struct_len_second)) < 0) {
        printf("Failed to accept second_client\n");
    }
    printf("Accept second client\n");
    char message_first[41]; // Сообщения от первого клиента.
    char message_second[42]; // Сообщения от второго клиента.
    // В цикле на протяжении 5 дней организуем работу полива.
    // Для начала рандомно определяем цветы, которые нужно будет полить.
    // Далее отправляем данные о поливке первому клиенту (садовнику), после второму клиенту (садовнику).
    // Т.е сервер отправляет сообщение с данными о поливке, а получает в ответ результат данного полива.
    for (int i = 0; i < 5; ++i) {
        num = 1 + rand() % (10 - 1);
        chooseFlowersForWatering(flowers, num);
        // Общение с первым садовником.
        write(connection_fd_first, flowers, sizeof(flowers));
        read(connection_fd_first, message_first, sizeof(message_first));
        printMessage(message_first, sizeof(message_first));
        sleep(2);
        // Общение со вторым садовником.
        write(connection_fd_second, flowers, sizeof(flowers));
        read(connection_fd_second, message_second, sizeof(message_second));
        printMessage(message_second, sizeof(message_second));
    }
    close(connection_fd_first);
}

