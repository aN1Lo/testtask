#pragma comment ( lib, "ws2_32.lib" )

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "stdafx.h"
#include <iostream>
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>

using namespace std;

#define MY_PORT 666 //порт, который слушает сервер, № 666

//макрос для печати количества активных пользователей
//#define PRINTNEWUSERS if (nclients == 1) printf("%d client on-line.\n", nclients); else if(nclients > 1) printf("%d clients on-line.\n", nclients); else printf("No clients on-line.\n");

//макрос для печати сообщения о подключении сокета к клиенту
#define sHELLO "Socket connected.\r\n"

//количество активных пользователей
int nclients = 0;

//функция для печати количества активных пользователей
void PRINTNEWUSERS()
{
	if (nclients == 1)
		printf("%d client on-line.\n", nclients);
	else if (nclients > 1)
		printf("%d clients on-line.\n", nclients);
	else
		printf("No clients on-line.\n");
}

//функция, создающаяся в отдельном потоке 
//и обсуживающая очередного подключившегося клиента независимо от остальных
DWORD WINAPI SetToClient(LPVOID client_socket)
{
	SOCKET my_sock;
	my_sock = ((SOCKET *)client_socket)[0];
	char buff[24];

	//отправляем клиенту приветственное сообщение
	send(my_sock, sHELLO, sizeof(sHELLO), 0);

	//цикл проверки клиента "на жизнь"
	//если можем писать ему, значит он живой
	while (true)
	{
		if (send(my_sock, &buff[0], 0, 0) == SOCKET_ERROR)
		{
			//если мы здесь, то соединение с клиентом разорвано
			nclients--; //уменьшаем счетчик активных клиентов
			printf("-- One client disconnected.\n");
			PRINTNEWUSERS();

			//закрываем сокет
			closesocket(my_sock);
			return 0;
		}
		Sleep(2000);
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	char buff[1024];

	printf("WatchDog server.\n");

	// Шаг 1 - Инициализация Библиотеки Сокетов
	//первым параметром задается версия библиотеки,
	//для старых версий задавалось $101, для новой версии (Winsock2) используется число $202, 
	//вторым параметром передается структура TWSAData(WSAData), в которую будет записана некоторая информация о библиотеке
	//таков прототип функции - int WSAStartup (  WORD wVersionRequested, LPWSADATA lpWSAData  )
	if (WSAStartup(0x0202, (WSADATA *)&buff[0]))
	{
		//Ошибка!
		printf("Error WSAStartup %d.\n", WSAGetLastError());
		return -1;
	}

	// Шаг 2 - создание сокета
	SOCKET mysocket;

	//AF_INET - сокет Интернета
	//SOCK_STREAM - потоковый сокет (с установкой соединения)
	//0 - по умолчанию выбирается TCP протокол
	if ((mysocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		//Ошибка!
		printf("Error socket %d\n", WSAGetLastError());
		WSACleanup(); //деиницилизация библиотеки Winsock
		return -1;
	}

	// Шаг 3 - связывание сокета с локальным адресом
	sockaddr_in local_addr;
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(MY_PORT); //не забываем о сетевом порядке
	local_addr.sin_addr.s_addr = 0; //сервер принимает подключения
	//на все свои IP-адреса

	//bind для связывания
	if (bind(mysocket, (sockaddr *)&local_addr, sizeof(local_addr)))
	{
		//Ошибка
		printf("Error bind %d\n", WSAGetLastError());
		closesocket(mysocket); //закрываем сокет
		WSACleanup();
		return -1;
	}

	// Шаг 4 - ожидание подключений
	//размер очереди - 0x100
	if (listen(mysocket, 0x100))
	{
		//Ошибка
		printf("Error listen %d\n", WSAGetLastError());
		closesocket(mysocket);
		WSACleanup();
		return -1;
	}

	printf("Waiting for connections...\n");

	// Шаг 5 - извлекаем сообщение из очереди
	SOCKET client_socket; //сокет для клиента
	sockaddr_in client_addr; //адрес клиента (заполняется системой)

	//функции accept необходимо передать размер структуры
	int client_addr_size = sizeof(client_addr);

	//цикл извлечения запросов на подключение из очереди
	while ((client_socket = accept(mysocket, (sockaddr *)&client_addr, &client_addr_size)))
	{
		nclients++; //увеличиваем счетчик подключившихся клиентов

		//пытаемся получить имя хоста
		HOSTENT *hst;
		hst = gethostbyaddr((char *)&client_addr.sin_addr.s_addr, 4, AF_INET);

		//вывод сведений о клиенте
		printf("-- %s [%s] now connected.\n",
			(hst) ? hst->h_name : "", inet_ntoa(client_addr.sin_addr));
		PRINTNEWUSERS();

		//Вызов нового потока для обслужвания клиента:
		//для этого рекомендуется использовать _beginthreadex но, 
		//поскольку никаких вызовов функций стандартной Си библиотеки поток не делает, 
		//можно обойтись и CreateThread
		DWORD thID;
		CreateThread(NULL, 0, SetToClient, &client_socket, 0, &thID);
	}

	return 0;
}

