#include "chat_server.hpp"

//初始化网络库
bool TcpChatServer::InitNetwork()
{
	WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "初始化Winsock失败" << WSAGetLastError() << endl;
        return false;
    }
    return true;
}

//创建套接字
SOCKET TcpChatServer::CreateSocket()
{
    SOCKET sock = socket(AF_INET,SOCK_STREAM,0);
    if (sock == INVALID_SOCKET) {
        cerr << "创建socket失败" << WSAGetLastError() << endl;
    }
    return sock;
}

// 配置地址
void TcpChatServer::ConfigSocketAddress(sockaddr_in& addr, int port) {
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
}

// 绑定端口
bool TcpChatServer::BindSocket() {
    sockaddr_in server_addr{};
    ConfigSocketAddress(server_addr, serverPort);

    if (bind(serverSocket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        cerr << "绑定失败" << WSAGetLastError() << endl;
        return false;
    }
    return true;
}


//开始监听
bool TcpChatServer::StartListen()
{
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "监听失败" << WSAGetLastError() << endl;
        return false;
    }
    cout << "监听成功，等待连接..." << endl;
    return true;
}

//接收客户端连接
void TcpChatServer::AcceptClients()
{
    while (isRunning) {
        sockaddr_in client_addr{};  // 使用client_addr来存储客户端地址
        int addrlen = sizeof(client_addr);

        SOCKET clientSocket = accept(serverSocket,(sockaddr*)&client_addr, &addrlen);
        if (clientSocket == INVALID_SOCKET) {
            if (isRunning) {
                cerr << "接收连接失败: " << WSAGetLastError() << endl;
            }
            continue;
        }

        //输出连接客户端信息
        //存储转换后的IP地址字符串
        char client_ip[INET_ADDRSTRLEN];//INET_ADDRSTRLEN 是一个预定义常量，表示存储IPv4地址字符串所需的最大字符数
        //将二进制IP地址转换为点分十进制表示法的字符串
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        cout << "客户端连接成功，IP: " << client_ip << "，端口: " << ntohs(client_addr.sin_port) << endl;

        //添加客户端连接到连接列表
        //使用作用域{}创建局部作用域，以便在添加到列表之后立即销毁
        {
            lock_guard<mutex> lock(clientsMutex);
            clientSockets.push_back(clientSocket);
        }

        //创建新进程处理客户端
        thread clientThread(&TcpChatServer::HandleClient, this, clientSocket);
        clientThreads.push_back(move(clientThread));
    }
}

void TcpChatServer::HandleClient(SOCKET clientSocket)
{
    //存储客户端名称
    char clientsName[32];
	memset(clientsName, 0, sizeof(clientsName));
	int receivedBytes = recv(clientSocket, clientsName, sizeof(clientsName) - 1, 0);//留一字节给结束符
    if (receivedBytes > 0) {
		string clientName(clientsName);
		{
			lock_guard<mutex> lock(clientsMutex);
			clientNames[clientSocket] = clientName;
		}
		cout << clientName << " 已加入聊天" << endl;
	}

    RecvMessage(clientSocket);
}

void TcpChatServer::RecvMessage(SOCKET clientSocket)
{
    char buff[4096];
    while (isRunning) {
        //清空缓冲区
        memset(buff, 0, sizeof(buff));
        int receivedBytes = recv(clientSocket, buff, sizeof(buff) - 1, 0);//留一字节给结束符
        if (!isRunning)break;
        if (receivedBytes <= 0) {
			string exitMessage;
            string username;
			{
				lock_guard<mutex> lock(clientsMutex);
				auto it = clientNames.find(clientSocket);
				if (it != clientNames.end()) {
                    username = it->second;
					exitMessage = username + " 已退出聊天";
					clientNames.erase(it);
				}
			}
            BroadcastMessage(exitMessage, clientSocket);

            if (receivedBytes == 0) {
                cout << "客户端 " << username << " 断开连接" << endl;
            }
            else {
                int error = WSAGetLastError();
                if (error == 10053) {
                    cout << "客户端 " << username << " 软件导致连接中止" << endl;
                }
                else if (error == 10054) {
                    cout << "客户端 " << username << " 断开连接" << endl;
                }
                else {
                    cerr << "接收消息失败: " << error << endl;
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
        string message(buff);
        cout <<  message << endl;
        BroadcastMessage(message, clientSocket);
    }
}

void TcpChatServer::SendMessage(SOCKET clientSocket, const string& message)
{
    if (send(clientSocket, message.c_str(), message.size(), 0) == SOCKET_ERROR) {
        cerr << "发送消息失败: " << WSAGetLastError() << endl;
    }
}

void TcpChatServer::BroadcastMessage(const string& message, SOCKET excludeSocket)
{
    lock_guard<mutex> lock(clientsMutex);
    for (const auto& socket : clientSockets) {
        if (socket != excludeSocket) {
            SendMessage(socket, message);
        }
    }
}

