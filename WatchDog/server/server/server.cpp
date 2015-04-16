#pragma comment ( lib, "ws2_32.lib" )

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "stdafx.h"
#include <iostream>
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>

using namespace std;

#define MY_PORT 666 //����, ������� ������� ������, � 666

//������ ��� ������ ���������� �������� �������������
//#define PRINTNEWUSERS if (nclients == 1) printf("%d client on-line.\n", nclients); else if(nclients > 1) printf("%d clients on-line.\n", nclients); else printf("No clients on-line.\n");

//������ ��� ������ ��������� � ����������� ������ � �������
#define sHELLO "Socket connected.\r\n"

//���������� �������� �������������
int nclients = 0;

//������� ��� ������ ���������� �������� �������������
void PRINTNEWUSERS()
{
	if (nclients == 1)
		printf("%d client on-line.\n", nclients);
	else if (nclients > 1)
		printf("%d clients on-line.\n", nclients);
	else
		printf("No clients on-line.\n");
}

//�������, ����������� � ��������� ������ 
//� ������������ ���������� ��������������� ������� ���������� �� ���������
DWORD WINAPI SetToClient(LPVOID client_socket)
{
	SOCKET my_sock;
	my_sock = ((SOCKET *)client_socket)[0];
	char buff[24];

	//���������� ������� �������������� ���������
	send(my_sock, sHELLO, sizeof(sHELLO), 0);

	//���� �������� ������� "�� �����"
	//���� ����� ������ ���, ������ �� �����
	while (true)
	{
		if (send(my_sock, &buff[0], 0, 0) == SOCKET_ERROR)
		{
			//���� �� �����, �� ���������� � �������� ���������
			nclients--; //��������� ������� �������� ��������
			printf("-- One client disconnected.\n");
			PRINTNEWUSERS();

			//��������� �����
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

	// ��� 1 - ������������� ���������� �������
	//������ ���������� �������� ������ ����������,
	//��� ������ ������ ���������� $101, ��� ����� ������ (Winsock2) ������������ ����� $202, 
	//������ ���������� ���������� ��������� TWSAData(WSAData), � ������� ����� �������� ��������� ���������� � ����������
	//����� �������� ������� - int WSAStartup (  WORD wVersionRequested, LPWSADATA lpWSAData  )
	if (WSAStartup(0x0202, (WSADATA *)&buff[0]))
	{
		//������!
		printf("Error WSAStartup %d.\n", WSAGetLastError());
		return -1;
	}

	// ��� 2 - �������� ������
	SOCKET mysocket;

	//AF_INET - ����� ���������
	//SOCK_STREAM - ��������� ����� (� ���������� ����������)
	//0 - �� ��������� ���������� TCP ��������
	if ((mysocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		//������!
		printf("Error socket %d\n", WSAGetLastError());
		WSACleanup(); //�������������� ���������� Winsock
		return -1;
	}

	// ��� 3 - ���������� ������ � ��������� �������
	sockaddr_in local_addr;
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(MY_PORT); //�� �������� � ������� �������
	local_addr.sin_addr.s_addr = 0; //������ ��������� �����������
	//�� ��� ���� IP-������

	//bind ��� ����������
	if (bind(mysocket, (sockaddr *)&local_addr, sizeof(local_addr)))
	{
		//������
		printf("Error bind %d\n", WSAGetLastError());
		closesocket(mysocket); //��������� �����
		WSACleanup();
		return -1;
	}

	// ��� 4 - �������� �����������
	//������ ������� - 0x100
	if (listen(mysocket, 0x100))
	{
		//������
		printf("Error listen %d\n", WSAGetLastError());
		closesocket(mysocket);
		WSACleanup();
		return -1;
	}

	printf("Waiting for connections...\n");

	// ��� 5 - ��������� ��������� �� �������
	SOCKET client_socket; //����� ��� �������
	sockaddr_in client_addr; //����� ������� (����������� ��������)

	//������� accept ���������� �������� ������ ���������
	int client_addr_size = sizeof(client_addr);

	//���� ���������� �������� �� ����������� �� �������
	while ((client_socket = accept(mysocket, (sockaddr *)&client_addr, &client_addr_size)))
	{
		nclients++; //����������� ������� �������������� ��������

		//�������� �������� ��� �����
		HOSTENT *hst;
		hst = gethostbyaddr((char *)&client_addr.sin_addr.s_addr, 4, AF_INET);

		//����� �������� � �������
		printf("-- %s [%s] now connected.\n",
			(hst) ? hst->h_name : "", inet_ntoa(client_addr.sin_addr));
		PRINTNEWUSERS();

		//����� ������ ������ ��� ����������� �������:
		//��� ����� ������������� ������������ _beginthreadex ��, 
		//��������� ������� ������� ������� ����������� �� ���������� ����� �� ������, 
		//����� �������� � CreateThread
		DWORD thID;
		CreateThread(NULL, 0, SetToClient, &client_socket, 0, &thID);
	}

	return 0;
}

