#include "chat_server.hpp"

// Initialize network library
bool TcpChatServer::InitNetwork()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Winsock initialization failed: " << WSAGetLastError() << endl;
        return false;
    }
    return true;
}

// Create socket
SOCKET TcpChatServer::CreateSocket()
{
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        cerr << "Socket creation failed: " << WSAGetLastError() << endl;
    }
    return sock;
}

// Configure address
void TcpChatServer::ConfigSocketAddress(sockaddr_in& addr, int port) {
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
}

// Bind port
bool TcpChatServer::BindSocket() {
    sockaddr_in server_addr{};
    ConfigSocketAddress(server_addr, serverPort);

    if (::bind(serverSocket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        cerr << "Binding failed: " << WSAGetLastError() << endl;
        return false;
    }
    return true;
}

// Start listening
bool TcpChatServer::StartListen()
{
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "Listening failed: " << WSAGetLastError() << endl;
        return false;
    }
    cout << "Listening successfully, waiting for connections..." << endl;
    return true;
}

// Accept client connections
void TcpChatServer::AcceptClients()
{
    while (isRunning) {
        sockaddr_in client_addr{};  // Use client_addr to store client address
        int addrlen = sizeof(client_addr);

        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&client_addr, &addrlen);
        if (clientSocket == INVALID_SOCKET) {
            if (isRunning) {
                cerr << "Connection acceptance failed: " << WSAGetLastError() << endl;
            }
            continue;
        }

        // Output connected client information
        // Store converted IP address string
        char client_ip[INET_ADDRSTRLEN]; // INET_ADDRSTRLEN is a predefined constant, indicating the maximum characters needed to store IPv4 address string
        // Convert binary IP address to dotted decimal notation string
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        cout << "Client connected successfully, IP: " << client_ip << ", Port: " << ntohs(client_addr.sin_port) << endl;

        // Add client connection to connection list
        // Use scope {} to create local scope, to destroy immediately after adding to the list
        {
            lock_guard<mutex> lock(clientsMutex);
            clientSockets.push_back(clientSocket);
        }

        // Create new process to handle client
        thread clientThread(&TcpChatServer::HandleClient, this, clientSocket);
        clientThreads.push_back(move(clientThread));
    }
}

void TcpChatServer::HandleClient(SOCKET clientSocket) {
    bool isLogin = false;
    while (!isLogin) {
        try {
            char buffer[1024];
            memset(buffer, 0, sizeof(buffer));

            // Receive client data
            int receivedBytes = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (receivedBytes <= 0) {
                if (receivedBytes == 0) {
                    cout << "Client actively disconnected" << endl;
                }
                else {
                    cerr << "Receiving error: " << WSAGetLastError() << endl;
                }
                closesocket(clientSocket);
                return;
            }

            // Parse JSON data
            json loginData;
            try {
                loginData = json::parse(buffer, buffer + receivedBytes);
            }
            catch (const json::parse_error& e) {
                json errorResponse;
                errorResponse["type"] = "InvalidFormat";
                errorResponse["message"] = "Invalid JSON format";
                sendJsonMessage(errorResponse, clientSocket);
                continue;
            }

            // Validate username format
            if (!loginData.contains("username") ||
                !loginData["username"].is_string() ||
                loginData["username"].get<string>().empty()) {
                json errorResponse;
                errorResponse["type"] = "InvalidUsername";
                errorResponse["message"] = "Username format error (3-16 alphanumeric characters)";
                sendJsonMessage(errorResponse, clientSocket);
                continue;
            }

            string username = loginData["username"];

            // Check for duplicate username
            {
                lock_guard<mutex> lock(clientsMutex);
                if (activeUsers.count(username)) {
                    json errorResponse;
                    errorResponse["type"] = "UsernameTaken";
                    errorResponse["message"] = "Username already taken, please try another";
                    sendJsonMessage(errorResponse, clientSocket);
                    continue;
                }

                activeUsers.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(username),
                    std::forward_as_tuple(username, clientSocket) // Direct construction, avoiding multiple copies
                );
            }

            // Login success handling
            json successResponse;
            successResponse["type"] = "announcement";
            successResponse["message"] = username + " has entered the chat room";
            BroadcastMessage(successResponse, clientSocket);
            isLogin = true;

        }
        catch (const exception& e) {
            cerr << "Client handling exception: " << e.what() << endl;
            closesocket(clientSocket);
            return;
        }
    }

    RecvMessage(clientSocket);
}

void TcpChatServer::RecvMessage(SOCKET clientSocket)
{
    char buff[MAX_MESSAGE_SIZE];
    while (isRunning) {
        // Clear buffer
        memset(buff, 0, sizeof(buff));
        int receivedBytes = recv(clientSocket, buff, sizeof(buff) - 1, 0); // Leave one byte for null terminator
        if (!isRunning) break;
        if (receivedBytes <= 0) {
            json exitMsg;
            string username;
            {
                lock_guard<mutex> lock(clientsMutex);
                for (const auto& client : activeUsers) {
                    if (client.second.getSocket() == clientSocket) {
                        username = client.first;
                        exitMsg["type"] = "announcement";
                        exitMsg["message"] = username + " has left the chat";
                        activeUsers.erase(client.first);
                        break;
                    }
                }
            }
            BroadcastMessage(exitMsg, clientSocket);

            if (receivedBytes == 0) {
                cout << "Client " << username << " disconnected" << endl;
            }
            else {
                int error = WSAGetLastError();
                if (error == 10053) {
                    cout << "Client " << username << " software caused connection abort" << endl;
                }
                else if (error == 10054) {
                    cout << "Client " << username << " disconnected" << endl;
                }
                else {
                    cerr << "Message receiving failed: " << error << endl;
                }
            }

            {
                lock_guard<mutex> lock(clientsMutex);
                auto pos = find(clientSockets.begin(), clientSockets.end(), clientSocket);
                if (pos != clientSockets.end()) {
                    clientSockets.erase(pos);
                }
            }
            closesocket(clientSocket);
            break;
        }

        // Parse JSON data
        try {
            json msgData = json::parse(buff, buff + receivedBytes);

            if (msgData.contains("type")) {
                // Check message type
                if (msgData["type"] == "GetUserList") {
                    sendUserList(clientSocket);
                    continue;
                }
                else if (msgData["type"] == "message") {
                    cout << buff << endl;
                    BroadcastMessage(msgData, clientSocket);
                }
            }
            else {
                cerr << "Invalid message format" << endl;
                continue;
            }
        }
        catch (const json::parse_error& e) {
            cerr << "JSON parsing failed: " << e.what() << endl;
            json errorResponse;
            errorResponse["type"] = "InvalidFormat";
            errorResponse["message"] = "Invalid JSON format";
            sendJsonMessage(errorResponse, clientSocket);
            continue;
        }
    }
}

void TcpChatServer::BroadcastMessage(const json& message, SOCKET excludeSocket)
{
    string jsonMsg = message.dump();
    size_t msgSize = jsonMsg.size();
    const char* msgData = jsonMsg.c_str();

    // Use vector to store sockets to remove
    vector<SOCKET> socketsToRemove;

    {
        lock_guard<mutex> lock(clientsMutex);

        for (const auto& clientSocket : clientSockets) {
            if (clientSocket != excludeSocket) {
                if (send(clientSocket, msgData, static_cast<int>(msgSize), 0) == SOCKET_ERROR) {
                    cerr << "Broadcasting message failed: " << WSAGetLastError() << endl;
                    socketsToRemove.push_back(clientSocket);
                }
            }
        }

        // Batch delete failed connections
        for (const auto& socketToRemove : socketsToRemove) {
            auto it = find(clientSockets.begin(), clientSockets.end(), socketToRemove);
            if (it != clientSockets.end()) {
                clientSockets.erase(it);
            }
        }
    }
}

// Send JSON message
bool TcpChatServer::sendJsonMessage(const json& jsonMsg, SOCKET clientSocket)
{
    string jsonString = jsonMsg.dump();
    lock_guard<mutex> lock(clientsMutex);
    if (send(clientSocket, jsonString.c_str(), static_cast<int>(jsonString.size()), 0) == SOCKET_ERROR)
    {
        cerr << "JSON message sending failed: " << WSAGetLastError() << endl;
        return false;
    }
    return true;
}

// Send user list
bool TcpChatServer::sendUserList(SOCKET clientSocket)
{
    json userList;
    userList["type"] = "UserList";
    for (const auto& user : activeUsers) {
        userList["users"].push_back(user.first);
    }
    sendJsonMessage(userList, clientSocket);
    return true;
}
