//============================================================================
// Name        : server_test.cpp
// Author      : Hemant Kapoor
// Version     :
// Copyright   : Your copyright notice
// Description : This is a simple server project
//============================================================================

#ifdef NEVER
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> //For sockaddr_in
#include <arpa/inet.h>  //For htonl
#include <unistd.h>     //for read
#include <functional>

using namespace std;

#define LISTEN_PORT 5000
#define NUMBER_OF_CONNECTION_ALLOWED 1
#define READ_BUFFER_SIZE 1024

void test()
{
	cout<<"Test\n";
}

#endif

#include <iostream>
#include "TcpServer.h"

int main() {
	std::cout << "!!!Hello World!!!" << std::endl; // prints !!!Hello World!!!

     TcpServer myServer;
     myServer.connect(5000);
     myServer.startListening(NULL);

     //sleep for 5 seconds
     while(true)
     {
         std::this_thread::sleep_for (std::chrono::seconds(5));
         std::cout<<"Main loop Running successful\n";
     }


#ifdef NEVER

	//function<void()> hello = [](){cout<<"Hello\n";};
	auto hello = [](){cout<<"Hello\n";};
	hello();

	//First this to do is to create a socket

	/*
	 * Prototype
	 * int socket(int domain, int type, int protocol);
	 * where
	 * domain = AF_INET - IPv4 Internet protocols
	 * type = SOCK_STREAM - Provides sequenced, reliable, two-way, connection-based byte  streams.
	 * protocol = TBD
	 * Return Socket File Descriptor
	*/
	int serverDescriptor = socket(AF_INET,SOCK_STREAM,0);
	std::cout<<"Creating socket...\n";

	if(serverDescriptor < 0)
	{
		std::cout<<"Unable to create socket...\n";
		return 1;
	}
	std::cout<<"Socket created...\n";

	//Once the socket is created we need to bind it to particular ip and port
	/*
	 * Prototype
	 * int bind(int sockfd, const struct sockaddr *addr,
	 *                socklen_t addrlen);
	 * Where
	 * sockfd : Socket file descriptor
	 * sockaddr : Address
	 * addrlen: Address length
	 */
	//Construct local address structure
	struct sockaddr_in server_address;
	//Clear the address
	memset(&server_address,0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	//server_address.sin_addr.in_addr = htonl(INADDR_ANY);
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(LISTEN_PORT);
	std::cout<<"Binding socket...\n";
	int bindResult = bind(serverDescriptor,(struct sockaddr*)&server_address,sizeof(server_address));
	if(bindResult < 0)
	{
		std::cout<<"Unable to bind socket...\n";
		return 2;
	}
	std::cout<<"Socket Bind completed...\n";

	//Now lets start listening
	std::cout<<"Listening...\n";
	listen(serverDescriptor,NUMBER_OF_CONNECTION_ALLOWED);

	//Lets create temporary read buffer
	char readBuffer[READ_BUFFER_SIZE];

	//Continous loop to accept connection
	while(true)
	{
		struct sockaddr clientSocket;
		socklen_t length;
		//Waiting to accept the client
		//Note this is a blocking call
		std::cout<<"Waiting for client to be connected...\n";
		int clientDescriptor = accept(serverDescriptor,&clientSocket,&length);

		//Once we are here, we get client connected
		std::cout<<"Client connected...\n";

		//Blocking read
		int readLength;
		do
		{
			memset(readBuffer,0,sizeof(readBuffer));
			readLength = read(clientDescriptor,readBuffer, READ_BUFFER_SIZE);
			std::cout<<"Read Length = " << readLength<<endl;
			readBuffer[readLength] = '\n';
			printf("Message received = %s\n",readBuffer);
		}while(readLength > 0);
		std::cout<<"Client disconnected \n";
		//Todo Print


	}

#endif
	return 0;
}
