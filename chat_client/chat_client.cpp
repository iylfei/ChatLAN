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

bool TcpChatClient::Connect()
{
    if (isConnected) return true;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(serverPort);
    
    if (inet_pton(AF_INET, serverIP.c_str(), &addr.sin_addr) <= 0) {
        std::cerr << "无效的IP地址" << std::endl;
        return false;
    }

    if (connect(serverSocket, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        cerr << "连接服务器失败" << WSAGetLastError() << endl;
        return false;
    }
    isConnected = true;
    cout << "成功连接到服务器" << "IP:" << serverIP << " Port:" << serverPort << endl;

    //发送用户名到服务器
	if (send(serverSocket, username.c_str(), username.size(),0) < 0) {
		cerr << "连接异常，请检查网络状况" << WSAGetLastError() << endl;
		isConnected = false;
		return false;
	}
    
    return true;
}

bool TcpChatClient::SendMessage(const string& message)
{
    string fullMessage = username + ": " + message;
    if (send(serverSocket, fullMessage.c_str(), fullMessage.size(), 0) == SOCKET_ERROR) {
        if (WSAGetLastError() != 10054) {
            cerr << "发送消息失败" << WSAGetLastError() << endl;
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
        if (!isConnected)break;
        if (receivedBytes > 0) {
            if (receivedBytes > MAX_MESSAGE_SIZE) {
                cerr << "接收消息长度超过上限" << endl;
                continue;
            }
            cout << buff << endl;
        }
        else {
            if (receivedBytes == 0) {
                cout << "服务器关闭了连接" << endl;
            }
            else if (WSAGetLastError() == 10054) {
                cout << "服务器已关闭" << endl;
            }
            else  {
                cerr << "消息接受错误" << WSAGetLastError() << endl;
            }
            break;
        }
    }
    isConnected = false;
}

//客户端私聊
