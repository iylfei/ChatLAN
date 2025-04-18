#ifndef CHAT_CLIENT_HPP
#define CHAT_CLIENT_HPP

#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <signal.h>
#include "json.hpp"
#include <winsock2.h>   // Windows Socket API (TCP/UDP基础)
#include <ws2tcpip.h>   // Windows Socket扩展API
#include <mswsock.h>    // Microsoft-specific扩展
#pragma comment(lib, "ws2_32.lib")  // 链接WinSock库

static const size_t MAX_MESSAGE_SIZE = 4096;

using namespace std;
using json = nlohmann::json;

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

	friend class CommandHandler;

private:
	SOCKET serverSocket;
	string username;
	bool isConnected;
	string serverIP;
	thread recvThread;
	int serverPort;
	mutex mtx;

	bool InitNetwork();
	SOCKET CreateSocket();
	bool Connect();
	void RecvMessage();
	bool SendMessage(const string& message);

};

	
#endif // ! chat_client