#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define sa struct sockaddr

int info_clients_count;
int len, socket_fd;
int working_clients[20];
struct sockaddr_in info_clients[20];
int current_day[1];
struct sockaddr_in first_client, second_client;
// Функция выводит сообщение, полученное от очередного клиента.
void printMessage(char *message, int size, int num) {
    if (num == -1) {
        for (int i = 0; i < size; ++i) {
            printf("%c", message[i]);
        }
    } else {
        printf("%d ", num);
        for (int i = 0; i < size - 4; ++i) {
            printf("%c", message[i]);
        }
    }
    printf("\n");
}

// Функция позволяет обрабатывать сигнал ctrl+c.
// При помощи неё есть возможность подключать и отключать инфо клиентов.
void disconnectingTheClient(int sig) {
    int watered_flowers[21], num;
    printf("If you want to disconnect info client write 1\n");
    printf("If you want to connect info client write 2\n");
    scanf("%d", &num);
    if (num == 1) {
        printf("Which client do you want to disconnect?\n");
        scanf("%d", &num);
        watered_flowers[0] = -1;
        sendto(socket_fd, watered_flowers, sizeof(watered_flowers), MSG_CONFIRM, (const struct sockaddr *) &info_clients[num - 1], len);
        working_clients[num - 1] = 0;
    } else if (num == 2) {
        int where;
        for (int i = 0; i < 20; ++i) {
            if (working_clients[i] == 0) {
                where = i;
                working_clients[i] = 1;
                break;
            }
        }
        printf("Launch info client, server is waiting...\n");
        char ready_message[45];
        current_day[0] += 1;
        recvfrom(socket_fd, ready_message, sizeof(ready_message), MSG_WAITALL, (struct sockaddr *) &info_clients[where], &len);
        printMessage(ready_message, sizeof(ready_message), where + 1);
        sendto(socket_fd, current_day, sizeof(current_day), MSG_CONFIRM, (const struct sockaddr *) &info_clients[where], len);
    }
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
    signal(SIGINT, disconnectingTheClient);
    info_clients_count = atoi(argv[2]);// Кол-во info клиентов.
    int num;
    int port = atoi(argv[1]);
    current_day[0] = 0;
    struct sockaddr_in servaddr;

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

    printf("Server is waiting for clients...\n");

    len = sizeof(servaddr);
    char ready_message[45];
    // Получаем сообщение от первого клиента о готовности обмена данными.
    recvfrom(socket_fd, ready_message, sizeof(ready_message), MSG_WAITALL, (const struct sockaddr *) &first_client, &len);
    printMessage(ready_message, sizeof(ready_message), -1);
    // Получаем сообщение от второго клиента о готовности обмена данными.
    recvfrom(socket_fd, ready_message, sizeof(ready_message), MSG_WAITALL, (const struct sockaddr *) &second_client, &len);
    printMessage(ready_message, sizeof(ready_message), -1);

    // Получаем сообщение о готовности подключения от каждого info клиента.
    // Далее посылаем очередному info клиенту день полива. Изначально это 0.
    // Если клиент после отключения будет подключаться снова, то ему важно будет знать текущий день полива.
    // Кол-во подключаемых инфо клиентов передаётся вторым аргументом командной строки.
    for (int i = 0; i < info_clients_count; ++i) {
        recvfrom(socket_fd, ready_message, sizeof(ready_message), MSG_WAITALL, (const struct sockaddr *) &info_clients[i], &len);
        sendto(socket_fd, current_day, sizeof(current_day), MSG_CONFIRM, (const struct sockaddr *) &info_clients[i], len);
        printMessage(ready_message, sizeof(ready_message), i + 1);
        working_clients[i] = 1;
    }

    int message_first[21]; // Сообщения от первого клиента.
    int message_second[21];// Сообщения от второго клиента.
    // В цикле на протяжении 5 дней организуем работу полива.
    // Для начала рандомно определяем цветы, которые нужно будет полить.
    // Далее отправляем данные о поливке первому клиенту (садовнику), после второму клиенту (садовнику).
    // Данные, которые сервер получает от первого и второго клиентов, отправляются info клиенту.
    // Info клиент отображает комплексную информацию.
    for (int i = 0; i < 5; ++i) {
        current_day[0] = i;
        num = 1 + rand() % (10 - 1);
        chooseFlowersForWatering(flowers, num);
        // Общение с первым садовником.
        sendto(socket_fd, flowers, sizeof(flowers), MSG_CONFIRM, (const struct sockaddr *) &first_client, len);
        recvfrom(socket_fd, message_first, sizeof(message_first), MSG_WAITALL, (struct sockaddr *) &first_client, &len);

        // Отправка info клиентам данных по первому садовнику.
        for (int j = 0; j < info_clients_count; ++j) {
            sendto(socket_fd, message_first, sizeof(message_first), MSG_CONFIRM, (const struct sockaddr *) &info_clients[j], len);
        }
        // Общение со вторым садовником.
        sendto(socket_fd, flowers, sizeof(flowers), MSG_CONFIRM, (const struct sockaddr *) &second_client, len);
        recvfrom(socket_fd, message_second, sizeof(message_second), MSG_WAITALL, (struct sockaddr *) &second_client, &len);
        sleep(2);
        // Отправка info клиентам данных по второму садовнику.
        for (int j = 0; j < info_clients_count; ++j) {
            sendto(socket_fd, message_second, sizeof(message_second), MSG_CONFIRM, (const struct sockaddr *) &info_clients[j], len);
        }
        sleep(2);
    }
}
