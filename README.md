# windows_dz1
Первое домашнее задание по windows.
Компиляция файлов:
gcc -o server server.c -lws2_32 -luser32 -lmswsock -ladvapi32 -mwindows
gcc -o client client.c -lws2_32 -luser32 -lmswsock -ladvapi32 -mwindows
Запускаем сначала сервер, а потом клиента с указанием ipv4 адресса:

    ./server
    ./client ip_v4_addr
