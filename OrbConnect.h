#ifndef _ORB_CONNECT_H
#define _ORB_CONNECT_H


#include <vector>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "windows.h"

#include "SimulinkOrbiterInterface.h"

#define DEFAULT_BUFLEN 1024
#define DEFAULT_PORT 37777
#define SUBSCRIPTION_DELAY 1

using namespace std;

typedef struct {
	sockaddr_in CLIaddr;
	int clientSize;
} CLIENT;

extern HANDLE clientListenThread;
extern DWORD dwID[];
extern vector<CLIENT> clients;

long WINAPI clientListenThreadFunction(long lParam);

bool startup();
void joinClient(sockaddr_in *client , int clientSize);
void subscribeData();
void sendData(long lParam,string Message);

#endif