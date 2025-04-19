#include "chat_client.hpp"

bool TcpChatClient::InitNetwork()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Init failed: " << WSAGetLastError() << endl;
        return false;
    }
    return true;
}

SOCKET TcpChatClient::CreateSocket()
{
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        cerr << "Create socket failed: " << WSAGetLastError() << endl;
    }
    return sock;
}

bool TcpChatClient::Connect()
{
    if (isConnected) return true;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(serverPort);

    if (inet_pton(AF_INET, serverIP.c_str(), &addr.sin_addr) <= 0) {
        std::cerr << "Invalid IP address" << std::endl;
        return false;
    }

    if (connect(serverSocket, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        cerr << "Connect to server failed: " << WSAGetLastError() << endl;
        return false;
    }
    isConnected = true;
    cout << "Successfully connected to server " << "IP:" << serverIP << " Port:" << serverPort << endl;

    while (username.empty()) {
        cout << "Please enter username: ";
        getline(cin, username);

        // Using regex to validate username
        regex usernamePattern("^[a-zA-Z0-9_]{3,16}$");

        if (!regex_match(username, usernamePattern)) {
            cout << "Invalid username! Requirements: 3-16 characters, only letters and numbers" << endl;
            username.clear();
        }
    }
    // Send login information
    json loginMsg;
    loginMsg["username"] = username;
    string sendData = loginMsg.dump();
    send(serverSocket, sendData.c_str(), static_cast<int>(sendData.size()), 0);
    return true;
}

bool TcpChatClient::SendChatMessage(const string& message)
{
    json messageJson;
    messageJson["type"] = "message";
    messageJson["message"] = username + ":" + message;
    string jsonMsg = messageJson.dump();
    if (send(serverSocket, jsonMsg.c_str(), static_cast<int>(jsonMsg.size()), 0) == SOCKET_ERROR) {
        if (WSAGetLastError() != 10054) {
            cerr << "Send message failed: " << WSAGetLastError() << endl;
        }
        isConnected = false;
        return false;
    }
    return true;
}

void TcpChatClient::RecvMessage()
{
    char buff[4096];
    while (isConnected) {
        memset(buff, 0, sizeof(buff));
        int receivedBytes = recv(serverSocket, buff, sizeof(buff) - 1, 0);
        if (!isConnected) break;
        if (receivedBytes > 0) {
            if (receivedBytes > MAX_MESSAGE_SIZE) {
                cerr << "Message length exceeds limit" << endl;
                continue;
            }
            try {
                json msgData = json::parse(buff, buff + receivedBytes);
                if (msgData.contains("type")) {
                    if (msgData["type"] == "UserList") {
                        cout << "Online users:" << endl;
                        for (const auto& user : msgData["users"]) {
                            cout << user.get<string>() << endl;
                        }
                    }
                    else if (msgData["type"] == "announcement" || msgData["type"] == "dm") {
                        cout << msgData["message"].get<string>() << endl;
                    }
                    else if (msgData["type"] == "InvalidFormat" || msgData["type"] == "InvalidUsername") {
                        cerr << "Server error: " << msgData["message"].get<string>() << endl;
                    }
                    else {
                        cerr << "Unknown message type" << endl;
                    }
                }
            }
            catch (const json::parse_error& e) {
                cerr << "JSON parsing failed: " << e.what() << endl;
                continue;
            }
        }
        else {
            if (receivedBytes == 0) {
                cout << "Server closed the connection" << endl;
            }
            else if (WSAGetLastError() == 10054) {
                cout << "Server has been closed" << endl;
            }
            else {
                cerr << "Message receiving error: " << WSAGetLastError() << endl;
            }
            break;
        }
    }
    isConnected = false;
}
