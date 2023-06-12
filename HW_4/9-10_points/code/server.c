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
int gardeners[2];
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
// При помощи неё есть возможность подключать и отключать инфо клиентов, садовников, а также корректно завершать работу сервера.
void disconnectingTheClient(int sig) {
    int watered_flowers[21], num;
    printf("If you want to stop server write -1\n");
    printf("If you want to disconnect info client write 1\n");
    printf("If you want to connect info client write 2\n");
    printf("If you want to disconnect gardeners write 3\n");
    printf("If you want to connect gardeners write 4\n");
    scanf("%d", &num);
    // Если хотим завершить работу сервера.
    if (num == -1) {
        char flowers[40];
        watered_flowers[0] = -3;
        flowers[0] = 's';
        for (int i = 0; i < info_clients_count; ++i) {
            sendto(socket_fd, watered_flowers, sizeof(watered_flowers), MSG_CONFIRM, (const struct sockaddr *) &info_clients[i], len);
        }
        sendto(socket_fd, flowers, sizeof(flowers), MSG_CONFIRM, (const struct sockaddr *) &first_client, len);
        sendto(socket_fd, flowers, sizeof(flowers), MSG_CONFIRM, (const struct sockaddr *) &second_client, len);
        printf("Server has stopped working\n");
        exit(0);
    }
    // Если хотим отключить какого-то информационного клиента.
    if (num == 1) {
        printf("Which client do you want to disconnect?\n");
        scanf("%d", &num);
        watered_flowers[0] = -1;
        sendto(socket_fd, watered_flowers, sizeof(watered_flowers), MSG_CONFIRM, (const struct sockaddr *) &info_clients[num - 1], len);
        working_clients[num - 1] = 0;
    } else if (num == 2) { // Если хотим подключить какого-то информационного клиента.
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

    else if (num == 3) { // Если хотим остановить работу какого-то садовника.
        char flowers[40];
        printf("Which gardener do you want to disconnect?\n");
        scanf("%d", &num);
        if (num == 1) {
            gardeners[0] = 0;
            flowers[0] = 'c';
            sendto(socket_fd, flowers, sizeof(flowers), MSG_CONFIRM, (const struct sockaddr *) &first_client, len);
        } else if (num == 2) {
            gardeners[1] = 0;
            flowers[0] = 'c';
            sendto(socket_fd, flowers, sizeof(flowers), MSG_CONFIRM, (const struct sockaddr *) &second_client, len);
        }
    } else if (num == 4) { // Если хотим возобновить работу какого-то садовника.
        current_day[0] += 1;
        printf("Which gardener do you want to connect?\n");
        scanf("%d", &num);
        if (num == 1) {
            printf("Launch first gardener, server is waiting...\n");
            gardeners[0] = 1;
            char ready_message[45];
            // Получаем сообщение от первого клиента о готовности обмена данными.
            recvfrom(socket_fd, ready_message, sizeof(ready_message), MSG_WAITALL, (const struct sockaddr *) &first_client, &len);
            printMessage(ready_message, sizeof(ready_message), -1);
            sendto(socket_fd, current_day, sizeof(current_day), MSG_CONFIRM, (const struct sockaddr *) &first_client, len);

        } else if (num == 2) {
            printf("Launch second gardener, server is waiting...\n");
            gardeners[1] = 1;
            char ready_message[45];
            // Получаем сообщение от первого клиента о готовности обмена данными.
            recvfrom(socket_fd, ready_message, sizeof(ready_message), MSG_WAITALL, (const struct sockaddr *) &second_client, &len);
            sendto(socket_fd, current_day, sizeof(current_day), MSG_CONFIRM, (const struct sockaddr *) &second_client, len);
            printMessage(ready_message, sizeof(ready_message), -1);
        }
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
    gardeners[0] = 1;
    gardeners[1] = 1;
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
    sendto(socket_fd, current_day, sizeof(current_day), MSG_CONFIRM, (const struct sockaddr *) &first_client, len);
    printMessage(ready_message, sizeof(ready_message), -1);
    // Получаем сообщение от второго клиента о готовности обмена данными.
    recvfrom(socket_fd, ready_message, sizeof(ready_message), MSG_WAITALL, (const struct sockaddr *) &second_client, &len);
    sendto(socket_fd, current_day, sizeof(current_day), MSG_CONFIRM, (const struct sockaddr *) &second_client, len);
    printMessage(ready_message, sizeof(ready_message), -1);

    // Получаем информацию о готовности подключения от каждого info клиента.
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
        // Если в массиве gardeners под 0 индексом лежит 0, то первый садовник отключён и отправлять ему ничего ну нужно.
        if (gardeners[0] != 0) {
            sendto(socket_fd, flowers, sizeof(flowers), MSG_CONFIRM, (const struct sockaddr *) &first_client, len);
            recvfrom(socket_fd, message_first, sizeof(message_first), MSG_WAITALL, (struct sockaddr *) &first_client, &len);
        }
        // Отправка info клиентам данных по первому садовнику.
        // Если первый садовник отключён, то в массив помещаем -2, чтобы информационные клиенты не выводили информацию о нём.
        if (gardeners[0] == 0) {
            message_first[0] = -2;
        }
        for (int j = 0; j < info_clients_count; ++j) {
            sendto(socket_fd, message_first, sizeof(message_first), MSG_CONFIRM, (const struct sockaddr *) &info_clients[j], len);
        }

        // Общение со вторым садовником.
        // Если в массиве gardeners под 1 индексом лежит 0, то второй садовник отключён и отправлять ему ничего ну нужно.
        if (gardeners[1] != 0) {
            sendto(socket_fd, flowers, sizeof(flowers), MSG_CONFIRM, (const struct sockaddr *) &second_client, len);
            recvfrom(socket_fd, message_second, sizeof(message_second), MSG_WAITALL, (struct sockaddr *) &second_client, &len);
        }
        if (gardeners[0] != 0) {
            sleep(2);
        }
        // Отправка info клиентам данных по второму садовнику.
        // Если второй садовник отключён, то в массив помещаем -2, чтобы информационные клиенты не выводили информацию о нём.
        if (gardeners[1] == 0) {
            message_second[0] = -2;
        }
        for (int j = 0; j < info_clients_count; ++j) {
            sendto(socket_fd, message_second, sizeof(message_second), MSG_CONFIRM, (const struct sockaddr *) &info_clients[j], len);
        }
        if (gardeners[1] != 0) {
            sleep(2);
        }
    }
}

