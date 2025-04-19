#ifndef CHAT_SERVER_HPP
#define CHAT_SERVER_HPP

#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <unordered_map>
#include "json.hpp"
#include "user.hpp"
#include <winsock2.h>   // Windows Socket API (TCP/UDP basics)
#include <ws2tcpip.h>   // Windows Socket extended API
#include <mswsock.h>    // Microsoft-specific extensions
#pragma comment(lib, "ws2_32.lib")  // Link WinSock library

static const size_t MAX_MESSAGE_SIZE = 4096;

using namespace std;
using json = nlohmann::json;

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

        // Create thread to accept connections
        acceptThread = thread(&TcpChatServer::AcceptClients, this);

        return true;
    }

    void stop() {
        static bool isstopped = false;
        if (isstopped) return;
        isstopped = true;
        cout << "Stopping server..." << endl;
        isRunning = false;

        if (serverSocket != INVALID_SOCKET) {
            cout << "Closing server socket..." << endl;
            closesocket(serverSocket);
            serverSocket = INVALID_SOCKET;
        }

        cout << "Waiting for accept thread to exit..." << endl;
        if (acceptThread.joinable()) {
            acceptThread.detach();
        }

        {
            cout << "Closing all client connections..." << endl;
            lock_guard<mutex> lock(clientsMutex);
            for (auto& sock : clientSockets) {
                closesocket(sock);
            }
            clientSockets.clear();
        }

        cout << "Waiting for client threads to exit..." << endl;
        for (auto& t : clientThreads) {
            if (t.joinable()) {
                t.detach();
            }
        }
        clientThreads.clear();

        cout << "Cleaning up Winsock resources..." << endl;
        WSACleanup();
    }
    friend class CommandHandler;

private:
    int serverPort;
    SOCKET serverSocket;
    atomic<bool> isRunning;
    thread acceptThread;
    vector<thread> clientThreads;
    vector<SOCKET> clientSockets;
    mutex clientsMutex;
    unordered_map<string, user> activeUsers;

private:
    bool InitNetwork();          // Initialize
    SOCKET CreateSocket();       // Create basic TCP socket
    void ConfigSocketAddress(sockaddr_in& addr, int port); // Configure address
    bool BindSocket();           // Bind port
    bool StartListen();          // Listen
    void AcceptClients();        // Accept
    void HandleClient(SOCKET clientSocket);     // Handle commands
    void RecvMessage(SOCKET clientSocket);      // Receive messages
    void BroadcastMessage(const json& message, SOCKET excludeSocket = INVALID_SOCKET);
    bool sendUserList(SOCKET clientSocket);
    bool sendJsonMessage(const json& jsonMsg, SOCKET clientSocket);
};

#endif
