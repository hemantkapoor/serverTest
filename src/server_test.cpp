//============================================================================
// Name        : server_test.cpp
// Author      : Hemant Kapoor
// Version     :
// Copyright   : Your copyright notice
// Description : This is a simple server project
//============================================================================

#include <iostream>
#include "TcpServer.h"

int main() {
	std::cout << "!!!Hello World!!!" << std::endl; // prints !!!Hello World!!!

     TcpServer myServer;
     myServer.connect(5000);
     bool res = myServer.startListening(NULL);

     if(res == false)
     {
    	 std::cout<<"Listening failed, exiting applicationn";
    	 return -1;
     }

     //sleep for 5 seconds
     while(true)
     {
         std::this_thread::sleep_for (std::chrono::seconds(5));
         std::cout<<"Main loop Running successful\n";
     }
	return 0;
}
