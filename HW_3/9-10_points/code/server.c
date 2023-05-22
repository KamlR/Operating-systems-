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

int current_day;
int info_clients_count;
int connection_fd_info[100];
struct sockaddr_in info_clients[100];
int struct_len_info[100];
int socket_fd;

// Данная функция вызывается по сигналу ctrl+c.
// Предназначена она для того, чтобы отключить info клиентов или же завершить работу сервера.
// Если введено -1, то сервер завершит свою работу, все клиенту соотв. тоже.
// Если введено значение от 1 до кол-ва info клиентов, то отключается указанный info клиент.
void disconnectingTheClient(int sig) {
    int watered_flowers[21];
    int info_number;
    printf("To disconnect info client write its number\n");
    printf("To stop server write -1\n");
    printf("To connect info client again write -2\n");
    scanf("%d", &info_number);
    // Отключение сервера.
    if (info_number == -1) {
        watered_flowers[0] = -1;
        for (int i = 0; i < info_clients_count; ++i) {
            if (connection_fd_info[i] == -100) {
                continue;
            }
            write(connection_fd_info[i], watered_flowers, sizeof(watered_flowers));
            close(connection_fd_info[i]);
        }
        printf("Server has stopped working\n");
        printf("All clients stopped working too\n");
        exit(0);
    }

    // Если хотим вновь подключить отключённого клиента.
    else if (info_number == -2) {
        int start_day[1];
        start_day[0] = current_day;
        for (int i = 0; i < info_clients_count; ++i) {
            if (connection_fd_info[i] == -100) {
                if ((connection_fd_info[i] = accept(socket_fd, (sa *) &info_clients[i], &struct_len_info[i])) < 0) {
                    printf("Failed to accept again info client\n");
                    exit(0);
                }
                write(connection_fd_info[i], start_day, sizeof(start_day));
                printf("Again accept info client\n");
                break;
            }
        }
    }
    // Отключение выбранного info клиента.
    else if (info_number > info_clients_count || info_number <= 0) {
        printf("There is no info client with this number\n");
    } else {
        watered_flowers[0] = -1;
        write(connection_fd_info[info_number - 1], watered_flowers, sizeof(watered_flowers));
        close(connection_fd_info[info_number - 1]);
        connection_fd_info[info_number - 1] = -100;
        printf("Info client number %d successfully disconnected\n", info_number);
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
    signal(SIGINT, disconnectingTheClient);
    srand(time(NULL));
    info_clients_count = atoi(argv[2]);// Кол-во info клиентов.
    int num;
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

    // Данный цикл предназначен для подключения заданного числа info клиентов.
    // Для каждого из них созданы свои массивы.
    int start_day[1];
    start_day[0] = 0;
    for (int i = 0; i < info_clients_count; ++i) {
        if ((connection_fd_info[i] = accept(socket_fd, (sa *) &info_clients[i], &struct_len_info[i])) < 0) {
            printf("Failed to accept %d info client\n", i + 1);
            exit(0);
        }
        write(connection_fd_info[i], start_day, sizeof(start_day));
        printf("Accept %d info client\n", i + 1);
    }

    int message_first[21]; // Сообщения от первого клиента.
    int message_second[21];// Сообщения от второго клиента.

    for (int i = 0; i < 5; ++i) {
        current_day = i;
        num = 1 + rand() % (10 - 1);
        chooseFlowersForWatering(flowers, num);
        // Общение с первым садовником.
        write(connection_fd_first, flowers, sizeof(flowers));
        read(connection_fd_first, message_first, sizeof(message_first));

        // В цикле отправляем каждому info клиенту информацию о 1 садовнике.
        // Проверка на -100 нужна для случая, если info клиент уже отключён от сервера.
        for (int j = 0; j < info_clients_count; ++j) {
            if (connection_fd_info[j] == -100) {
                continue;
            }
            write(connection_fd_info[j], message_first, sizeof(message_first));
        }
        sleep(2);
        // Общение со вторым садовником.
        write(connection_fd_second, flowers, sizeof(flowers));
        read(connection_fd_second, message_second, sizeof(message_second));

        // В цикле отправляем каждому info клиенту информацию о 2 садовнике.
        // Проверка на -100 нужна для случая, если info клиент уже отключён от сервера.
        for (int j = 0; j < info_clients_count; ++j) {
            if (connection_fd_info[j] == -100) {
                continue;
            }
            write(connection_fd_info[j], message_second, sizeof(message_second));
        }
        sleep(2);
    }
    close(connection_fd_first);
    close(connection_fd_second);
    close(socket_fd);
    int watered_flowers[21];
    watered_flowers[0] = -1;
    for (int i = 0; i < info_clients_count; ++i) {
        if (connection_fd_info[i] == -100) {
            continue;
        }
        write(connection_fd_info[i], watered_flowers, sizeof(watered_flowers));
        close(connection_fd_info[i]);
    }
}

