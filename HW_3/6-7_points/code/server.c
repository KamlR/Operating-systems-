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
    int connection_fd_info, struct_len_info;
    int port = atoi(argv[1]);
    struct sockaddr_in servaddr, first_client, second_client, info_client;
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

    // Установка входящего соединения с клиентом, который отображает комплексную информацию.
    if ((connection_fd_info = accept(socket_fd, (sa *) &info_client, &struct_len_info)) < 0) {
        printf("Failed to accept info_client\n");
    }
    printf("Accept info client\n");
    int message_first[21]; // Сообщения от первого клиента.
    int message_second[21]; // Сообщения от второго клиента.
    // В цикле на протяжении 5 дней организуем работу полива.
    // Для начала рандомно определяем цветы, которые нужно будет полить.
    // Далее отправляем данные о поливке первому клиенту (садовнику), после второму клиенту (садовнику).
    // Данные, которые сервер получает от первого и второго клиентов, отправляются info клиенту.
    // Info клиент отображает комплексную информацию.
    for (int i = 0; i < 5; ++i) {
        num = 1 + rand() % (10 - 1);
        chooseFlowersForWatering(flowers, num);
        // Общение с первым садовником.
        write(connection_fd_first, flowers, sizeof(flowers));
        read(connection_fd_first, message_first, sizeof(message_first));

        // Отправка info клиенту данных по первому садовнику.
        write(connection_fd_info, message_first, sizeof(message_first));

        // Общение со вторым садовником.
        write(connection_fd_second, flowers, sizeof(flowers));
        read(connection_fd_second, message_second, sizeof(message_second));
        sleep(2);
        // Отправка info клиенту данных по второму садовнику.
        write(connection_fd_info, message_second, sizeof(message_second));
        sleep(2);
    }
    close(connection_fd_first);
}
