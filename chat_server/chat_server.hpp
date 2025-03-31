#ifndef CHAT_SERVER_HPP
#define CHAT_SERVER_HPP

#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <winsock2.h>   // Windows Socket API (TCP/UDP����)
#include <ws2tcpip.h>   // Windows Socket��չAPI
#include <mswsock.h>    // Microsoft-specific��չ
#pragma comment(lib, "ws2_32.lib")  // ����WinSock��

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

        // �����������ӵ��߳�
        acceptThread = thread(&TcpChatServer::AcceptClients, this);

        return true;
    }

    void stop() {
        static bool isstopped = false;
        if (isstopped)return;
        isstopped = true;
        cout << "����ֹͣ������..." << endl;
        isRunning = false;

        if (serverSocket != INVALID_SOCKET) {
            cout << "�رշ�����socket..." << endl;
            closesocket(serverSocket);
            serverSocket = INVALID_SOCKET;
        }

        cout << "�ȴ�accept�߳��˳�..." << endl;
        if (acceptThread.joinable()) {
            acceptThread.detach();
        }

        {
            cout << "�ر����пͻ�������..." << endl;
            lock_guard<mutex> lock(clientsMutex);
            for (auto& sock : clientSockets) {
                closesocket(sock);
            }
            clientSockets.clear();
        }

        cout << "�ȴ��ͻ����߳��˳�..." << endl;
        for (auto& t : clientThreads) {
            if (t.joinable()) {
                t.detach();
            }
        }
        clientThreads.clear();

        cout << "����Winsock��Դ..." << endl;
        WSACleanup();
    }

    void BroadcastMessage(const string& message, SOCKET excludeSocket = INVALID_SOCKET);

private:
	int serverPort;
	SOCKET serverSocket;
	atomic<bool> isRunning;
    thread acceptThread;
	vector<thread> clientThreads;
	vector<SOCKET> clientSockets;
	mutex clientsMutex; 
private:
	bool InitNetwork();          // ��ʼ��
	SOCKET CreateSocket();          // ��������TCP�׽���
    void ConfigSocketAddress(sockaddr_in& addr, int port);//���õ�ַ
	bool BindSocket();  // �󶨶˿�
	bool StartListen();		//����
	void AcceptClients();		//����
    void HandleClient(SOCKET clientSocket);
    void RecvMessage(SOCKET clientSocket);
    void SendMessage(SOCKET clientSocket, const string& message);
};

#endif