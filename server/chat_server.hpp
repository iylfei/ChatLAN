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

static const size_t MAX_MESSAGE_SIZE = 4096;

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

        if (!BindSocket()) {
            closesocket(serverSocket);
            return false;
        }

        if (!StartListen()) {
            closesocket(serverSocket);
            return false;
        }

        isRunning = true;

        // 创建接受连接的线程
        acceptThread = thread(&TcpChatServer::AcceptClients, this);

        return true;
    }

    void stop() {
        static bool isstopped = false;
        if (isstopped)return;
        isstopped = true;
        cout << "正在停止服务器..." << endl;
        isRunning = false;

        if (serverSocket != INVALID_SOCKET) {
            cout << "关闭服务器socket..." << endl;
            closesocket(serverSocket);
            serverSocket = INVALID_SOCKET;
        }

        cout << "等待accept线程退出..." << endl;
        if (acceptThread.joinable()) {
            acceptThread.detach();
        }

        {
            cout << "关闭所有客户端连接..." << endl;
            lock_guard<mutex> lock(clientsMutex);
            for (auto& sock : clientSockets) {
                closesocket(sock);
            }
            clientSockets.clear();
        }

        cout << "等待客户端线程退出..." << endl;
        for (auto& t : clientThreads) {
            if (t.joinable()) {
                t.detach();
            }
        }
        clientThreads.clear();

        cout << "清理Winsock资源..." << endl;
        WSACleanup();
    }

private:
	int serverPort;
	SOCKET serverSocket;
	atomic<bool> isRunning;
    thread acceptThread;
	vector<thread> clientThreads;
	vector<SOCKET> clientSockets;
	mutex clientsMutex; 
private:
	bool InitNetwork();          // 初始化
	SOCKET CreateSocket();          // 创建基础TCP套接字
    void ConfigSocketAddress(sockaddr_in& addr, int port);//配置地址
	bool BindSocket();  // 绑定端口
	bool StartListen();		//监听
	void AcceptClients();		//接收
    void HandleClient(SOCKET clientSocket);
    void RecvMessage(SOCKET clientSocket);
    void SendMessage(SOCKET clientSocket, const string& message);
    void BroadcastMessage(const string& message, SOCKET excludeSocket = INVALID_SOCKET);
};

#endif