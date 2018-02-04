/*
 * TcpServer.cpp
 *
 *  Created on: 3 Feb 2018
 *      Author: hemant
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> //For sockaddr_in
#include <arpa/inet.h>  //For htonl
#include <unistd.h>     //for read
#include <fcntl.h>      //for fnctl
#include <errno.h>      // for errno
#include <iostream>
#include <string.h>
#include <chrono>


#include "TcpServer.h"


bool TcpServer::connect(int portNumber)
{

	m_serverDescriptor = socket(AF_INET,SOCK_STREAM,0);
	std::cout<<"Creating socket...\n";

	if(m_serverDescriptor <= 0)
	{
		std::cout<<"Unable to create socket...\n";
		return false;
	}
	std::cout<<"Socket created...\n";
	//Construct local address structure
	struct sockaddr_in server_address;
	//Clear the address
	memset(&server_address,0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	//server_address.sin_addr.in_addr = htonl(INADDR_ANY);
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(portNumber);
	std::cout<<"Binding socket...\n";

	int bindResult = bind(m_serverDescriptor,(struct sockaddr*)&server_address,sizeof(server_address));
	if(bindResult < 0)
	{
		std::cout<<"Unable to bind socket...\n";
		return false;
	}
	std::cout<<"Socket Bind completed...\n";

	m_socketCreated = true;
	return true;
}

bool TcpServer::startListening(callback functionToCall)
{
	std::cout<<"Listening...\n";
	//if(!m_socketCreated || functionToCall == NULL)
	if(!m_socketCreated)
	{
		std::cout<<"Error Socket not created\n";
		return false;
	}
	listen(m_serverDescriptor,m_maxConnection);

	while (!m_stopListening)
	{
		struct sockaddr clientSocket;
		socklen_t length;
		//Waiting to accept the client
		//Note this is a blocking call
		std::cout<<"Waiting for client to be connected...\n";
		int clientDescriptor = accept(m_serverDescriptor,&clientSocket,&length);

		//Once we are here, we get client connected
		std::cout<<"Client connected...\n";

		//New thread for the client
		m_threadStarted = true;
		m_thread = std::thread(&TcpServer::clientThread,this,clientDescriptor);
		m_thread.join();
	}

	return true;
}

void TcpServer::clientThread(int clientDescriptor)
{
	char readBuffer[m_maxBufferSize];
	std::cout<<"Client connected \n";

	//Set the client socket to be non blocking
	fcntl(clientDescriptor, F_SETFL, O_NONBLOCK); /* Change the socket into non-blocking state	*/
	int readLength;
	bool continueReading(true);
	do
	{
		memset(readBuffer,0,sizeof(readBuffer));

		//Todo: This is a blocking read... Need unblocking read
		readLength = read(clientDescriptor,readBuffer, m_maxBufferSize);
		//check if there was nothing to read then don't bail out
		if(readLength < 0 && errno == EAGAIN)
		{
			//Lets sleep for 1 second before retrying
			std::cout<<"Got nothing sleeping for a second\n";
			std::this_thread::sleep_for(std::chrono::seconds(5));
		}
		if(readLength > 0)
		{
			std::cout<<"Read Length = " << readLength<<std::endl;
			readBuffer[readLength] = '\n';
			printf("Message received = %s\n",readBuffer);
		}
		else
		{
			std::cout<<"Client disconnected\n";
			continueReading = false;
		}
	}while(continueReading);
	std::cout<<"Exiting thread as client disconected\n";
}

TcpServer::~TcpServer() {
	// TODO Auto-generated destructor stub
}

