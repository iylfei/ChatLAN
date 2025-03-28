#ifndef CHAT_CLIENT_HPP
#define CHAT_CLIENT_HPP

#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <signal.h>
#include <winsock2.h>   // Windows Socket API (TCP/UDP����)
#include <ws2tcpip.h>   // Windows Socket��չAPI
#include <mswsock.h>    // Microsoft-specific��չ
#pragma comment(lib, "ws2_32.lib")  // ����WinSock��

static const size_t MAX_MESSAGE_SIZE = 4096;

using namespace std;

class TcpChatClient 
{
public:
	TcpChatClient(const string& username, const string& serverIP, int serverPort) :
		username(username), serverIP(serverIP), serverPort(serverPort),isConnected(false){}

	~TcpChatClient() {
		stop();
	}

	bool start() {
		if (!InitNetwork())return false;
		serverSocket = CreateSocket();
		if (serverSocket == SOCKET_ERROR)return false;
		if(!Connect())return false;

		recvThread = thread(&TcpChatClient::RecvMessage, this);

		return true;
	}

	void stop() {
		isConnected = false;
		if (serverSocket != INVALID_SOCKET) {
			closesocket(serverSocket);
			serverSocket = INVALID_SOCKET;
		}

		if (recvThread.joinable()) {
			recvThread.join();
		}

		WSACleanup();
	}
	bool SendMessage(const string& message);

private:
	bool isConnected;
	SOCKET serverSocket;
	string username;
	string serverIP;
	thread recvThread;
	int serverPort;

	bool InitNetwork();
	SOCKET CreateSocket();
	bool Connect();
	void RecvMessage();
};

	

#endif // ! chat_client