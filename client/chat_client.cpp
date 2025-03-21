#include "chat_client.hpp"

bool TcpChatClient::InitNetwork()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "初始化失败" << WSAGetLastError() << endl;
        return false;
    }
    return true;
}

SOCKET TcpChatClient::CreateSocket()
{
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        cerr << "创建socket失败" << WSAGetLastError() << endl;
    }
    return sock;
}

bool TcpChatClient::Connect(SOCKET sock, string serverIP,int port)
{
    if (isConnected) return true;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, serverIP.c_str(), &addr.sin_addr) <= 0) {
        std::cerr << "无效的IP地址" << std::endl;
        return false;
    }

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        cerr << "连接服务器失败" << WSAGetLastError() << endl;
        return false;
    }
    isConnected = true;
    cout << "成功连接到服务器" << "IP:" << serverIP << "Port:" << port << endl;
    return true;
}
