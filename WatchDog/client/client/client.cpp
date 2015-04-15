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

	// ��� 1 - ������������� ���������� Winsock
	if (WSAStartup(0x202, (WSADATA *)&buff[0]))
	{
		printf("WSAStart error %d.\n", WSAGetLastError());
		return -1;
	}

	// ��� 2 - �������� ������
	SOCKET my_sock;
	my_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (my_sock < 0)
	{
		printf("Socket() error %d.\n", WSAGetLastError());
		return -1;
	}

	// ��� 3 - ��������� ����������
	//���������� ��������� sockaddr_in - �������� ������ � ����� �������
	sockaddr_in dest_addr;
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(PORT);
	HOSTENT *hst;

	//�������������� IP ������ �� ����������� � ������� ������
	if (inet_addr(SERVERADDR) != INADDR_NONE)
		dest_addr.sin_addr.s_addr = inet_addr(SERVERADDR);
	else
	{
		//������� �������� IP ����� �� ��������� ����� �������
		if (hst = gethostbyname(SERVERADDR))
			//hst->h_addr_list �������� �� ������ �������,
			//� ������ ���������� �� ������
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

	//����� ������� ������� - �������� ���������� ����������
	if (connect(my_sock, (sockaddr *)&dest_addr, sizeof(dest_addr)))
	{
		printf("Connect error %d.\n", WSAGetLastError());
		return -1;
	}

	printf("Connection with %s is successful.\n \
		       Press any key for quit...\n\n", SERVERADDR);

	while (!_kbhit()) //���� ������� ����� �������
	{
	}

	//���������� �����
	printf("Exit.");
	Sleep(1000); //��������� �����
	closesocket(my_sock); //��������� �����
	WSACleanup();
	return 0;
}