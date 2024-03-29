//Simple TCP/IP client example.
//Connects to google via HTTP and requests the default page.

#include "stdafx.h"
#include<stdio.h>
#include<winsock2.h>
#pragma warning(disable:4996)

#define RESPONSE_SIZE 5000

int main(int argc, char *argv[])
{
	WSADATA wsa;
	SOCKET s;
	struct sockaddr_in server;
	const char *message;
	char server_reply[RESPONSE_SIZE];
	int recv_size;

	//Initialize Winsock.
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}
	printf("Initialised.\n");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Error: Could not create socket. Code=%d\n", WSAGetLastError());
		return -1;
	}
	printf("Socket created.\n");

	//Fill in address structure.
	server.sin_addr.s_addr = inet_addr("216.58.214.78"); //address of google.com
	server.sin_family = AF_INET;
	server.sin_port = htons(80);  // HTTP port

								  //Connect to remote server
	if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		printf("Error: couldn't connect. Code=%d\n", WSAGetLastError());
		return -1;
	}
	printf("Connected\n");

	//Send request to server
	message = "GET / HTTP/1.1\r\n\r\n";
	if (send(s, message, strlen(message), 0) < 0)
	{
		printf("Error: send failed. Code=%d\n", WSAGetLastError());
		return -1;
	}
	printf("Data Sent\n");

	//Receive a reply from the server
	if ((recv_size = recv(s, server_reply, RESPONSE_SIZE, 0)) == SOCKET_ERROR)
	{
		printf("Error: recv failed. Code=%d\n", WSAGetLastError());
		return -1;
	}
	printf("Reply received. Bytes received = %d\n", recv_size);

	//Add a NULL terminating character to make it a proper string before printing
	server_reply[recv_size] = '\0';

	//print out response
	printf(server_reply);

	//close socket
	closesocket(s);

	//cleanup winsock
	WSACleanup();

	return 0;
}