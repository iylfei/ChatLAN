#ifndef CHAT_CLIENT_HPP
#define CHAT_CLIENT_HPP

#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <regex>
#include <signal.h>
#include "json.hpp"
#include <winsock2.h>   // Windows Socket API (TCP/UDP basics)
#include <ws2tcpip.h>   // Windows Socket extension API
#include <mswsock.h>    // Microsoft-specific extensions
#pragma comment(lib, "ws2_32.lib")  // Link WinSock library

static const size_t MAX_MESSAGE_SIZE = 4096;

using namespace std;
using json = nlohmann::json;

class TcpChatClient
{
public:
	TcpChatClient(const string& serverIP, int serverPort) :
		serverIP(serverIP), serverPort(serverPort), isConnected(false) {
	}

	~TcpChatClient() {
		stop();
	}

	bool start() {
		if (!InitNetwork())return false;
		serverSocket = CreateSocket();
		if (serverSocket == SOCKET_ERROR)return false;
		if (!Connect())return false;

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
	bool SendChatMessage(const string& message);
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

};


#endif // ! chat_client
