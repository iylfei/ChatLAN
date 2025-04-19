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

    while (username.empty()) {
        cout << "请输入用户名: ";
        getline(cin, username);

        // 使用正则表达式验证用户名
        regex usernamePattern("^[a-zA-Z0-9_]{3,16}$");

        if (!regex_match(username, usernamePattern)) {
            cout << "用户名无效！要求：3-16个字符，只能包含字母和数字" << endl;
            username.clear();
        }
    }
    // 发送登录信息
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
            try {
				json msgData = json::parse(buff, buff + receivedBytes);
				if (msgData.contains("type")) {
					if (msgData["type"] == "UserList") {
						cout << "在线用户列表:" << endl;
						for (const auto& user : msgData["users"]) {
							cout << user.get<string>() << endl;
						}
					}
					else if (msgData["type"] == "announcement" || msgData["type"] == "dm") {
						cout << msgData["message"].get<string>() << endl;
					}
					else if (msgData["type"] == "InvalidFormat" || msgData["type"] == "InvalidUsername") {
						cerr << "服务器返回错误: " << msgData["message"].get<string>() << endl;
					}
                    else {
                        cerr << "未知消息类型" << endl;
                    }
				}
			}
            catch (const json::parse_error& e) {
                cerr << "解析JSON消息失败: " << e.what() << endl;
                continue;
            }
        }
        else {
            if (receivedBytes == 0) {
                cout << "服务器关闭了连接" << endl;
            }
            else if (WSAGetLastError() == 10054) {
                cout << "服务器已关闭" << endl;
            }
            else  {
                cerr << "消息接收错误" << WSAGetLastError() << endl;
            }
            break;
        }
    }
    isConnected = false;
}
