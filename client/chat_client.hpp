#ifndef CHAT_CLIENT_HPP
#define CHAT_CLIENT_HPP

#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <winsock2.h>   // Windows Socket API (TCP/UDP基础)
#include <ws2tcpip.h>   // Windows Socket扩展API
#include <mswsock.h>    // Microsoft-specific扩展
#pragma comment(lib, "ws2_32.lib")  // 链接WinSock库

using namespace std;

class TcpChatClient 
{
public:
	TcpChatClient(const string& username, const string& serverIP = "127.0.0.1", int serverPort = 8888) :
		username(username), serverIP(serverIP), serverPort(serverPort),isConnected(false){}

	~TcpChatClient() {
		stop();
	}

	bool start() {

	}

	void stop() {

	}

private:
	bool isConnected;
	string username;
	string serverIP;
	int serverPort;

	bool InitNetwork();
	SOCKET CreateSocket();
	bool Connect(SOCKET sock, string serverIP, int port);
};

	

#endif // ! chat_client