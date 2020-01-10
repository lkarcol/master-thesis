#define ORBITER_MODULE 

#include "OrbConnect.h"

HINSTANCE g_hDLL; 
HANDLE StartupThread;
HANDLE clientListenThread;
HANDLE pauseThread;
HANDLE startupThread;
HANDLE clientMutex;
DWORD dwID[3];
SOCKET listener;

WSADATA wsaData;
SOCKET listenSocket = INVALID_SOCKET;
SOCKET clientSocket = INVALID_SOCKET;

vector<CLIENT> clients;
struct addrinfo *result = NULL, hints;

int  iResult;
int  recvbuflen = DEFAULT_BUFLEN;
char* port = DEFAULT_PORT;

int lastTimeSlot;
SO_Interface soInterface;

int SendText_Intercept = 0;
string SendText_Buf;

// Callbec called after orbiter start
DLLCLBK void opcOpenRenderViewport (HWND renderWnd,DWORD width,DWORD height,BOOL fullscreen)
{ 
    if (startup())
    {
        oapiWriteLog("OrbConnect::Module Initialized");
    }
}

// Callback called every orbiter frame
DLLCLBK void opcPostStep(double SimT, double SimDT, double mjd)
{
	subscribeData();
}

void subscribeData() {

	if (clients.size() < 1) return;

	// Aktualny simulacny cas [s]
	double st = oapiGetSysTime();

	if (st >= lastTimeSlot + SUBSCRIPTION_DELAY) {

		for (int clientIdx = 0; clientIdx < clients.size(); clientIdx++)
		{
			sendData(clientIdx, soInterface.subscribeData());
		}
		oapiWriteLog("TEST");
		lastTimeSlot = st;
	}

}

// Create TCP server and THREAD for client
bool startup()
{

    char msgBuf[DEFAULT_BUFLEN];

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);

    if (iResult != 0)
    {
        sprintf_s(msgBuf, "OrbConnect::WSAStartup failed with accepted socket: %d", WSAGetLastError());
        oapiWriteLog(msgBuf);
        return false;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, port, &hints, &result);
    if ( iResult != 0 )
    {
        sprintf_s(msgBuf, "OrbConnect::getaddrinfo with accepted socket: %d", WSAGetLastError());
        oapiWriteLog(msgBuf);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    listener = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (listener == INVALID_SOCKET)
    {
        sprintf_s(msgBuf, "OrbConnect::socket failed with accepted socket: %d", WSAGetLastError());
        oapiWriteLog(msgBuf);
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }
    // Setup the TCP listening socket
    iResult = bind( listener, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        sprintf_s(msgBuf, "OrbConnect::bind failed with accepted socket: %d", WSAGetLastError());
        oapiWriteLog(msgBuf);
        freeaddrinfo(result);
        closesocket(listener);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(listener, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        sprintf_s(msgBuf, "OrbConnect::listen failed with accepted socket: %d", WSAGetLastError());
        oapiWriteLog(msgBuf);
        closesocket(listener);
        WSACleanup();
        return 1;
    }

    clientListenThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)clientListenThreadFunction, NULL, 0, &dwID[0]);
	soInterface.vesselInfo();

    return true;
}

// Send data to client over TCP
void sendData(long lParam, string Message)
{
	if (SendText_Intercept) {
		SendText_Buf = Message;
		return;
	}

	int length = Message.length();

	char SendText[1024];
	strcpy_s(SendText, Message.c_str());

	send(clients.at(lParam).TCPClient, SendText, length, 0);
}


long WINAPI clientListenThreadFunction(long lParam)
{
    char msgBuf[DEFAULT_BUFLEN];
    CLIENT newClient;
    // Accept a client socket
    while ((clientSocket = accept(listener, NULL, NULL)) != INVALID_SOCKET)
    {
        memset(&newClient, 0, sizeof(CLIENT));
        newClient.TCPClient = clientSocket;
        clientSocket = INVALID_SOCKET;

        if (newClient.TCPClient == SOCKET_ERROR)
        {
            sprintf_s(msgBuf, "OrbConnect::Error with accepted socket: %d", WSAGetLastError());
            oapiWriteLog(msgBuf);

            return 1;
        }

        char mutexId[32];
        sprintf_s(mutexId, "client%d", clients.size());
        newClient.mutex = CreateMutex(NULL, false, mutexId);

        clients.push_back(newClient);

        clients.at(clients.size() - 1).ClientReceiveThread =  CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ClientReceiveThreadFunction, LPVOID(clients.size() - 1), 0, &dwID[1]);
    }

    if (clientSocket == INVALID_SOCKET) {
        sprintf_s(msgBuf, "OrbConnect::accept failed: %d", WSAGetLastError());
        oapiWriteLog(msgBuf);
        closesocket(listener);
        WSACleanup();

        return 1;
    }

    return 0;
}

long WINAPI ClientReceiveThreadFunction(long lParam)
{
    int iResult;
    char recvbuf[DEFAULT_BUFLEN];
    char msgBuf[DEFAULT_BUFLEN];

    // Process incoming messages while there are incoming messages available
    while ((iResult = recv(clients.at(lParam).TCPClient, recvbuf, recvbuflen, 0)) > 0)
    {
        WaitForSingleObject(clients.at(lParam).mutex, INFINITE);

		// Print Command to console
		oapiWriteLog(recvbuf);

		string command = recvbuf;
		soInterface.processComands(command);


        ReleaseMutex(clients.at(lParam).mutex);

        memset(recvbuf,0,DEFAULT_BUFLEN);
    }

    if (iResult == 0)
    {
        oapiWriteLog("OrbConnect::recv returned 0; closing");
        closesocket(clients.at(lParam).TCPClient);
        clients.at(lParam).TCPClient = INVALID_SOCKET;
        return 1;
    }
    else
    {
        sprintf_s(msgBuf, "OrbConnect::recv failed for client %d with error %d; closing", lParam, WSAGetLastError());
        oapiWriteLog(msgBuf);
        closesocket(clients.at(lParam).TCPClient);
        clients.at(lParam).TCPClient = INVALID_SOCKET;
        return 1;
    }

    return 0;
}
