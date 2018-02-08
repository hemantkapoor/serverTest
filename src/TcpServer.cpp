/*
 * TcpServer.cpp
 *
 *  Created on: 3 Feb 2018
 *      Author: hemant
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> //For sockaddr_in
#include <arpa/inet.h>  //For htonl
#include <unistd.h>     //for read
#include <fcntl.h>      //for fnctl
#include <errno.h>      // for errno
#include <signal.h>
#include <poll.h>
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

	//New thread for the client
	m_threadStarted = true;
	m_thread = std::thread(&TcpServer::clientThread,this);
	//m_thread.join();

	return true;
}

void TcpServer::clientThread()
{
	bool displayClientMessage(true);
	//Lets make server socket to be polled
	struct pollfd serverPoll;
	serverPoll.fd = m_serverDescriptor;
	/*
	 * Events are :
	 * POLLIN     : There is data to read
	 */
	serverPoll.events = POLLIN;

	//Ready for client
	struct sockaddr clientSocket;
	socklen_t length;
	char readBuffer[m_maxBufferSize];
	int readLength;
	bool continueReading(true);
	/*Set the client descriptor polling on read */
	struct pollfd clientReadPoll;
	int clientDescriptor;
	/*
	 * Events are :
	 * POLLIN     : There is data to read
	 */
	clientReadPoll.events = POLLIN;

	while (!m_stopListening)
	{
		if(displayClientMessage)
		{
			displayClientMessage = false;
			std::cout<<"Waiting for Client to be connected\n";
		}
		int pollReturn =  poll(&serverPoll, 1, 3000);

		if(pollReturn == 0)
		{
			//Lets try again
			continue;
		}

		clientDescriptor = accept(m_serverDescriptor,&clientSocket,&length);
		clientReadPoll.fd = clientDescriptor;
		//Once we are here, we get client connected
		std::cout<<"Client connected...\n";

		while(continueReading && !m_stopListening)
		{
			//Lets have the timeout to be 5 second
			pollReturn =  poll(&clientReadPoll, 1, 5000);

			if(pollReturn == 0)
			{
				//Lets try again
				continue;
			}

			//Check which event occurred
			if(clientReadPoll.revents == POLLIN)
			{
				readLength = read(clientDescriptor,readBuffer, m_maxBufferSize);
				if(readLength == 0)
				{
					std::cout<<"Client disconnected\n";
					continueReading = false;
				}
				else if(readLength > 0)
				{
					readBuffer[readLength] = 0;
					//Convert to string as we need to pass this finally
					std::string receivedMessage(readBuffer);
					std::cout<<"Message received = "<<receivedMessage<<std::endl;
				}
			}
		}//End of client connection while loop
		displayClientMessage = true;
	}//End of main thread
}

TcpServer::~TcpServer() {
	// TODO Auto-generated destructor stub
	if(m_threadStarted)
	{
		std::cout<<"Waiting for thread to be closed\n";
		//Stop the thread
		m_stopListening = true;
		m_thread.join();
		std::cout<<"Thread closed...\n";
	}
}

