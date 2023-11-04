﻿#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#if defined(_MSC_VER) || defined(__BORLANDC__)
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#endif

#define DEFAULT_BUFLEN 4096
#define DEFAULT_PORT "29015"
#define INET_EXIT_STR "exit\n"

int __cdecl main(int argc, char** argv) {
    WSADATA wsaData; // Метаданные сетевых возможностей
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints; // Ноды в связном списке, нужные для создания ip сокета
    char sendbuf[DEFAULT_BUFLEN];
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;

    // Validate the parameters
    if (argc != 2) {
        printf("usage: %s server-name\n", argv[0]);
        return 1;
    }

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData); // Загрузка библиотеки с версией 2.2, и просто служебная структура
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints)); // memset нулями 
    hints.ai_family = AF_UNSPEC; // Неопределённое семейство протоколов
    hints.ai_socktype = SOCK_STREAM; // Самый простой вид взаимодействия с сокетом (ip + port)
    hints.ai_protocol = IPPROTO_TCP; // Спецификация протокола tcp для сетевого взаимодействия

    // Resolve the server address and port
    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result); // ��������� ���������� ��� ������� ������ (ip + port) ������������ � result
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        exit(1);
    }
    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) { // Пробуем в цикле подключится ко всем сокетам в листе

        // Create a SOCKET for connecting to server
        
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol); // �������� ������
        
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %d\n", WSAGetLastError());
            WSACleanup();
            exit(1);
        }
        // Connect to server.
        
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen); // ������������ �� ������
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
        }
    }

    freeaddrinfo(result); // Очищаем вспомогательную инфу

    
    if (ConnectSocket == INVALID_SOCKET) {
        printf("socket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        exit(1);
    }
    printf("Print: \"exit\"  to end session\n");
    // Receive until the peer closes the connection
    do { // Основной цикл обмена сообщениями
        // Send buffer
        for (int i = 0; ; ++i) { // Считывание символов с клавиатуры
            unsigned char c = (unsigned char)_getch();
            sendbuf[i] = c;
            if (i == DEFAULT_BUFLEN - 3) {
                sendbuf[i + 1] = '\n';
                sendbuf[i + 1] = '\0';
                printf("\nNOTE: Reached max length of bufer\n");
                break;
            }
            if (c == '\r') { // Перенос строки
                printf("\n");
                sendbuf[i] = '\n';
                sendbuf[i + 1] = '\0';
                break;
            }
            else if (c == '\b' && i > 0) { // Delete 
                sendbuf[i] = '\0';
                sendbuf[i - 1] = '\0';
                i -= 2;
                printf("\n%s", sendbuf);
                continue;
            }
            printf("%c", c);
        }
       
        iResult = send(ConnectSocket, sendbuf, DEFAULT_BUFLEN, 0); // ����� ������� �� ������������� ������
        if (iResult == SOCKET_ERROR) {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            exit(1);
        }
        printf("Bytes Sent: %d\n", iResult);
        // Получаем данные по сокету и записываем в буфер
        
        iResult = recv(ConnectSocket, recvbuf, DEFAULT_BUFLEN, 0);
        if (iResult > 0)
            printf("Bytes received: %d\n", iResult);
        else if (iResult == 0)
            printf("Connection closed\n");
        else
            printf("recv failed with error: %d\n", WSAGetLastError());
        printf("\n%s", recvbuf);
    } while (iResult > 0 && (strcmp(recvbuf, INET_EXIT_STR) != 0)); // работаем пока сервер не подтвердит выход

    // shutdown the connection since no more data will be sent
    iResult = shutdown(ConnectSocket, SD_SEND); // Отключения соединения сокетов
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();
    return 0;
}