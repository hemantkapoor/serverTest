/*
 * TcpServer.h
 *
 *  Created on: 3 Feb 2018
 *      Author: hemant
 */

#ifndef TCPSERVER_H_
#define TCPSERVER_H_

#include <thread>

//using callback = std::function<void(char*)>;
//typedef void(*callback)(char*);
using callback = std::function<void(std::string&)>;


class TcpServer {
public:
	TcpServer() = default;
	bool connect(int portNumber);
	//bool startListening(std::function<void(char*)>);
	bool startListening(callback);
	virtual ~TcpServer();

private:
	std::thread m_thread;
	int m_serverDescriptor=0;
	bool m_socketCreated = false;
	bool m_stopListening = false;
	bool m_threadStarted = false;  //This will be used when socket thread is non blocking...
	callback m_callBack = NULL;

	const int m_maxConnection = 1;
	const int m_maxBufferSize = 1024;

	void clientThread();

};

#endif /* TCPSERVER_H_ */
