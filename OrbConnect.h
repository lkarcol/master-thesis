#ifndef _ORB_CONNECT_H
#define _ORB_CONNECT_H


#include <vector>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "windows.h"

#include "SimulinkOrbiterInterface.h"

#define DEFAULT_BUFLEN 1024
#define DEFAULT_PORT "37777"
#define SUBSCRIPTION_DELAY 1

using namespace std;

typedef struct {
	SOCKET TCPClient;
	sockaddr * CLIaddr;
	HANDLE ClientReceiveThread,CommandProcessor;
	HANDLE mutex;
	string ReceiveBuffer;
	vector<string> CommandQueue;
} CLIENT;


extern HINSTANCE g_hDLL; 
extern HANDLE StartupThread;
extern HANDLE clientListenThread;
extern HANDLE pauseThread;
extern HANDLE startupThread;
extern HANDLE clientMutex;
extern DWORD dwID[];
extern SOCKET listener;
extern vector<CLIENT> clients;
extern WSADATA wsaData;
extern SOCKET listenSocket;
extern SOCKET clientSocket;

extern struct addrinfo *result,hints;

extern char* port;
extern int  iResult;
extern int  recvbuflen;

long WINAPI clientListenThreadFunction(long lParam);
long WINAPI ClientReceiveThreadFunction(long lParam);


bool startup();
void subscribeData();
void sendData(long lParam,string Message);

extern int SendText_Intercept;
extern string SendText_Buf;

#endif