#ifndef CHAT_SERVER_HPP
#define CHAT_SERVER_HPP

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

class TcpChatServer
{
public:
	TcpChatServer(int port) : serverPort(port), serverSocket(INVALID_SOCKET), isRunning(false) {}

	~TcpChatServer() {
		stop();
	}

    bool start() {
        if (!InitNetwork()) return false;

        serverSocket = CreateSocket();
        if (serverSocket == INVALID_SOCKET) return false;

        if (!BindSocket(serverSocket, serverPort)) {
            closesocket(serverSocket);
            return false;
        }

        if (!StartListen(serverSocket)) {
            closesocket(serverSocket);
            return false;
        }

        isRunning = true;

        // 创建接受连接的线程
        thread acceptThread(&TcpChatServer::AcceptClients, this);
        acceptThread.detach(); // 分离线程，让它在后台运行

        return true;
    }

    void stop() {
        isRunning = false;

        // 关闭所有客户端socket
        {
            lock_guard<mutex> lock(clientsMutex);
            for (auto& sock : clientSockets) {
                closesocket(sock);
            }
            clientSockets.clear();
        }

        // 关闭服务器socket
        if (serverSocket != INVALID_SOCKET) {
            closesocket(serverSocket);
            serverSocket = INVALID_SOCKET;
        }

        // 等待所有线程完成
        for (auto& t : clientThreads) {
            if (t.joinable()) t.join();
        }
        clientThreads.clear();

        WSACleanup();
    }

private:
	int serverPort;
	SOCKET serverSocket;
	atomic<bool> isRunning;
	vector<thread> clientThreads;
	vector<SOCKET> clientSockets;
	mutex clientsMutex; 
private:
	bool InitNetwork();          // 初始化
	SOCKET CreateSocket();          // 创建基础TCP套接字
    void ConfigSocketAddress(sockaddr_in& addr, int port);//配置地址
	bool BindSocket(SOCKET sock, int port);  // 绑定端口
	bool StartListen(SOCKET sock);		//监听
	void AcceptClients();		//接收
    void HandleClient(SOCKET clientSocket);
    void RecvMessage(SOCKET clientSocket);
    void SendMessage(SOCKET clientSocket, const string& message);
    void BroadcastMessage(const string& message, SOCKET excludeSocket = INVALID_SOCKET);
};

#endif