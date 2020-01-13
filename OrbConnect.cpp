#define ORBITER_MODULE 

#include "OrbConnect.h"
#include "utils.h";


HANDLE clientListenThread;
DWORD dwID[3];
SOCKET in;
vector<CLIENT> clients;
SO_Interface soInterface;

int lastTimeSlot;

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

		lastTimeSlot = st;
	}

}

// Create TCP server and THREAD for client
bool startup()
{

	// Start winsock
	WSADATA data;
	WORD version = MAKEWORD(2, 2);
	int wsOk = WSAStartup(version, &data);

	if (wsOk != 0) {
		oapiWriteLog("Cant start winsock");
		return 0;
	}

	// Bind socket tp port and adress
	in = socket(AF_INET, SOCK_DGRAM, 0);
	sockaddr_in serverHint;

	serverHint.sin_addr.S_un.S_addr = ADDR_ANY;
	serverHint.sin_family = AF_INET;
	serverHint.sin_port = htons(DEFAULT_PORT);

	if (bind(in, (sockaddr*)&serverHint, sizeof(serverHint)) == SOCKET_ERROR) {
		//cout << "Cant bind socket" << WSAGetLastError() << endl;
		oapiWriteLog("Cant bind socket");
		return 0;
	}

    clientListenThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)clientListenThreadFunction, NULL, 0, &dwID[0]);

	//	WSACleanup();
    return true;
}

// Send data to client over UDP
void sendData(long lParam, string Message)
{
	int length = Message.length();

	if (length < 1) return;

	char SendText[1024];
	strcpy_s(SendText, Message.c_str());

	sendto(in, SendText, length, 0 , (sockaddr*)&clients.at(lParam).CLIaddr, clients.at(lParam).clientSize);
}


void joinClient(sockaddr_in *client , int clientSize) {

	CLIENT newCLient;
	ZeroMemory(&newCLient, sizeof(newCLient));

	newCLient.CLIaddr = *client;
	newCLient.clientSize = clientSize;

	clients.push_back(newCLient);
	oapiWriteLog("New client connected");


	char b[100];
	int p = clients.size();
	sprintf_s(b, "Pocet klientov: %d", p);
	oapiWriteLog(b);

}

long WINAPI clientListenThreadFunction(long lParam)
{

	sockaddr_in client;
	int clientSize = sizeof(client);

	char buffer[DEFAULT_BUFLEN];
	vector<string> command;

	while (true) {

		ZeroMemory(&buffer, DEFAULT_BUFLEN);
		ZeroMemory(&client, clientSize);
		ZeroMemory(&command, sizeof(command));

		int receiveInitialMessage = recvfrom(in, buffer, DEFAULT_BUFLEN, 0, (sockaddr*)&client, &clientSize);

		if (receiveInitialMessage == SOCKET_ERROR) {
			oapiWriteLog("Error receive from client");
			continue;
		}
		else {

			stringSplit(buffer,"|",command);

			// NC = Command for NewClient
			if (command.at(0) == "NC") {
				joinClient(&client, clientSize);
			}
			else {
				soInterface.processComands(command);
			}
		}

	}
    return 0;
}
