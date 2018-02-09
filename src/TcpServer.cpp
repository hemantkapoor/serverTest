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
	m_callBack = functionToCall;

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

	/*Set the client descriptor polling on read */
	struct pollfd clientReadPoll;
	/*
	 * Events are :
	 * POLLIN     : There is data to read
	 */
	clientReadPoll.events = POLLIN;

	while (!m_stopListening)
	{
		bool continueReading(true);
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

		m_clientDescriptor = accept(m_serverDescriptor,&clientSocket,&length);
		clientReadPoll.fd = m_clientDescriptor;
		//Once we are here, we get client connected
		std::cout<<"Client connected...\n";
		/*
		 * Below code is to take into account when the connect and
		 * a message was sent immediately.
		 * We do a non blocking read to see any message is there before polling
		 */
		int saved_flags = fcntl(m_clientDescriptor, F_GETFL);
		// Set the new flags with O_NONBLOCK masked out
		fcntl(m_clientDescriptor, F_SETFL, saved_flags & ~O_NONBLOCK);

		readMessage(false);
		fcntl(m_clientDescriptor, F_SETFL, saved_flags);

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
				continueReading = readMessage();
			}
		}//End of client connection while loop
		m_clientDescriptor = 0;
		displayClientMessage = true;
	}//End of main thread
}

//Helper function to read message
bool TcpServer::readMessage(bool bIgnoreError /*=false*/)
{
	char readBuffer[m_maxBufferSize];
	auto readLength = read(m_clientDescriptor,readBuffer, m_maxBufferSize);
	if(readLength == 0 && !bIgnoreError)
	{
		std::cout<<"Client disconnected\n";
		return false;
	}
	else if(readLength > 0)
	{
		readBuffer[readLength] = 0;
		//Convert to string as we need to pass this finally
		std::string receivedMessage(readBuffer);
		std::cout<<"Message received = "<<receivedMessage<<std::endl;
		sendMessage("Hello");
		//Do the callback
		if(m_callBack != NULL)
		{
			m_callBack(receivedMessage);
		}
	}
	return true;

}

bool TcpServer::sendMessage(const std::string& message)
{
	if(m_clientDescriptor == 0)
	{
		std::cout<<"Cannot write and no client connected\n";
		return false;
	}
	//We do blocking write here... Don't think it will be an issue
	auto lengthWritten = write(m_clientDescriptor,(const void*)message.c_str(),message.length());

	if(lengthWritten < 0)
	{
		std::cout<<"Error with connection, cannot send message";
		return false;
	}
	//Below should go in a while loop
	if(static_cast<unsigned int>(lengthWritten) !=message.length())
	{
		std::cout<<"Error only "<<lengthWritten <<" is sent rather than "<<message.length()<<std::endl;
		return false;
	}
	else
	{
		std::cout<<"Message Sent\n";
	}
	//Everything is fine
	return true;

}

TcpServer::~TcpServer() {
	if(m_threadStarted)
	{
		std::cout<<"Waiting for thread to be closed\n";
		//Stop the thread
		m_stopListening = true;
		m_thread.join();
		std::cout<<"Thread closed...\n";
	}
}

