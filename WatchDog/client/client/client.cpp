#pragma comment ( lib, "ws2_32.lib" )

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "stdafx.h"
#include <stdio.h>
#include <conio.h>
#include <string.h> 
#include <winsock2.h> 
#include <windows.h> 

#define PORT 666 
#define SERVERADDR "127.0.0.1" 

int _tmain(int argc, _TCHAR* argv[])
{
	char buff[BUFSIZ];
	printf("WatchDog client.\n");

	// Шаг 1 - инициализация библиотеки Winsock
	if (WSAStartup(0x202, (WSADATA *)&buff[0]))
	{
		printf("WSAStart error %d.\n", WSAGetLastError());
		return -1;
	}

	// Шаг 2 - создание сокета
	SOCKET my_sock;
	my_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (my_sock < 0)
	{
		printf("Socket() error %d.\n", WSAGetLastError());
		return -1;
	}

	// Шаг 3 - установка соединения
	//заполнение структуры sockaddr_in - указание адреса и порта сервера
	sockaddr_in dest_addr;
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(PORT);
	HOSTENT *hst;

	//преобразование IP адреса из символьного в сетевой формат
	if (inet_addr(SERVERADDR) != INADDR_NONE)
		dest_addr.sin_addr.s_addr = inet_addr(SERVERADDR);
	else
	{
		//попытка получить IP адрес по доменному имени сервера
		if (hst = gethostbyname(SERVERADDR))
			//hst->h_addr_list содержит не массив адресов,
			//а массив указателей на адреса
			((unsigned long *)&dest_addr.sin_addr)[0] =
			((unsigned long **)hst->h_addr_list)[0][0];
		else
		{
			printf("Invalid address %s.\n", SERVERADDR);
			closesocket(my_sock);
			WSACleanup();
			return -1;
		}
	}

	//адрес сервера получен - пытаемся установить соединение
	if (connect(my_sock, (sockaddr *)&dest_addr, sizeof(dest_addr)))
	{
		printf("Connect error %d.\n", WSAGetLastError());
		return -1;
	}

	printf("Connection with %s is successful.\n \
		       Press any key for quit...\n\n", SERVERADDR);

	while (!_kbhit()) //ждем нажатия любой клавиши
	{
	}

	//корректный выход
	printf("Exit.");
	Sleep(1000); //небольшая пауза
	closesocket(my_sock); //закрываем сокет
	WSACleanup();
	return 0;
}