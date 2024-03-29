// UDP_ServerClient.cpp : Defines the entry point for the console application.
//
//Thomas Gilman
//Networking
//ETEC 3201
//1
//Lab 9 UDP DGRAMS
//CLIENT!!!!!!!!!!!!!!!!!!
#include "stdafx.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <windows.h>
#include <conio.h>

#pragma comment(lib, "ws2_32.lib") //include lib

#define BUFF_SIZE 144
#define SERVER_PORT 5020

char stringCheckBuff[9];
int clientOrServer = 0;

typedef struct CHAT_MSG
{
	unsigned short message_type; //1=join, 2=leave, 3=message
	char user_name[16];
	char user_message[128];
};

//wait loop for reading window shell
void wait()
{
	while (1)
	{
		printf("waiting, type continue to continue\n");
		fgets(stringCheckBuff, sizeof(stringCheckBuff), stdin);
		if (strcmp(stringCheckBuff, "continue\n"))
			break;
	}
}

void setAddress(struct sockaddr_in &address, ADDRESS_FAMILY family, unsigned short port, PCWSTR netAddress)
{
	memset(&address, 0, sizeof(address));					//fill padding with 0's
	address.sin_family = family;							//address family
	address.sin_port = htons(port);							//htons is host to network short port
	InetPton(AF_INET, netAddress, &address.sin_addr.s_addr);//puts into bytes destination IP

	if (address.sin_addr.s_addr < 0)
	{
		printf("Failed to pass netAddress!\n");
		wait();
		exit(-1);
	}
}

int main(int argc, char **argv)
{
	WSADATA wsa; //windows specific
	SOCKET sock_fd;
	PCWSTR IPaddress = L"127.0.0.1"; //my IPv4
	struct sockaddr_in serverAddress;
	struct sockaddr_in addressIn;
	socklen_t addrlen = sizeof(addressIn);

	CHAT_MSG message;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d\n", WSAGetLastError());
		wait();
		WSACleanup();
		return -1;
	}printf("Winsock Initialised!\n");
	if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("Error: Could not create socket. Code=%d\n", WSAGetLastError());
		wait();
		closesocket(sock_fd);
		WSACleanup();
		return -1;
	}printf("Socket created!\n");
	unsigned long mode = 1;
	if (ioctlsocket(sock_fd, FIONBIO, &mode) != 0)
	{
		printf("failed to set socket to nonBlocking. Code=%d\n", WSAGetLastError());
		wait();
		closesocket(sock_fd);
		WSACleanup();
		return -1;
	}
	//setUp Server addr
	setAddress(serverAddress, AF_INET, SERVER_PORT, IPaddress);

	memset(message.user_message, 0, sizeof(message.user_message));
	while (1)
	{
		printf("if you want to join a server please tpye JOIN, otherwise type EXIT to quite:\n");
		fgets(message.user_message, 127, stdin);
		if (strcmp(message.user_message, "JOIN"))		//Join Server
		{
			message.message_type = 1;
			break;
		}
		else if (strcmp(message.user_message, "EXIT")) //Exit program
		{
			closesocket(sock_fd);
			WSACleanup();
			exit(1);		
		}
	}
	printf(message.user_message);
	printf("Enter a user name of 16 characters or less:\n");
	fgets(message.user_name, 15, stdin);
	for (int index = 0; index < sizeof(message.user_name); index++)	//null terminate string instead of newline then null
	{
		if (message.user_name[index] == '\n')
			message.user_name[index] = ':';
	}
	for (int index = 0; index < sizeof(message.user_message); index++)	//null terminate string instead of newline then null
	{
		if (message.user_message[index] == '\n')
			message.user_message[index] = '\0';
	}
	printf(message.user_name);
	
	char messageToSend[143]; memset(messageToSend, 0, sizeof(messageToSend));
	strcat_s(messageToSend, sizeof(messageToSend), message.user_name);
	strcat_s(messageToSend, sizeof(messageToSend), message.user_message);
	printf("message being sent: %s\n", messageToSend);

	//join server
	if (sendto(sock_fd, messageToSend, strlen(messageToSend), 0, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
	{
		printf("could not send join message. ERROR CODE:%d\n", WSAGetLastError());
		wait();
		return -1;
	}
	memset(message.user_message, 0, sizeof(message.user_message));
	memset(messageToSend, 0, sizeof(messageToSend));
	char recv_buffer[BUFF_SIZE];
	memset(recv_buffer, 0, sizeof(recv_buffer));
	int recv_size;
	int charindex = 0;
	int joining = 0;
	printf("you are now in the server\n");
	//sending loop
	while (1)
	{
		if (_kbhit())
		{
			char charin = _getch();
			if (((int)charin > 47 && (int)charin < 92) || ((int)charin > 92 && (int)charin < 127)		//add character except special chars to message buffer
				|| ((int)charin > 47 && (int)charin < 58) || ((int)charin > 31 && (int)charin < 37)
				|| ((int)charin > 37 && (int)charin < 47))
			{
				printf("%c", charin);
				message.user_message[charindex++] = charin;
			}
			else if ((int)charin == 13)	//Carriage feed
			{
				printf("\n");
				
				message.user_message[charindex + 1] = '\0';												//null terminate message
				charindex = 0;																			//reset message char insert index to 0
				strcat_s(messageToSend, sizeof(messageToSend), message.user_name);
				strcat_s(messageToSend, sizeof(messageToSend), message.user_message);

				if (strcmp(message.user_message, "EXIT") == 0)											//user wants to exit
				{
					if (sendto(sock_fd, messageToSend, strlen(messageToSend), 0,
						(struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
						printf("could not send EXIT message. ERROR CODE:%d\n", WSAGetLastError());
					break;
				}
				if (sendto(sock_fd, messageToSend, strlen(messageToSend), 0,
					(struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
					printf("could not send message. ERROR CODE:%d\n", WSAGetLastError());
				printf("Message sent: %s\n", messageToSend);
				memset(message.user_message, 0, sizeof(message.user_message));							//clear array for next message
				memset(messageToSend, 0, sizeof(messageToSend));
			}
		}
		else if((recv_size = recvfrom(sock_fd, recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr *)&addressIn, &addrlen)) > 0)	//Recieveing a message
		{
			//message In Variables
			int messIndex = 0;

			while (recv_buffer[messIndex] != '\0' || messIndex == BUFF_SIZE - 1) //print message
			{
				printf("%c", recv_buffer[messIndex]);
				messIndex++;
			}
			printf("\n");
			memset(recv_buffer, 0, sizeof(recv_buffer));
		}
	}

	closesocket(sock_fd); //windows specific
						  //shutdown(sock_fd, 2); //UNIX specific: dont use sock_fd, for both reading/writing
	WSACleanup();
	printf("Sockets closed and winsock cleaned up.\n");

	return 0;
}

