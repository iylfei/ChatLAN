#include "chat_client.hpp"

atomic<bool> isrunning(true);

void signalHandler(int signal) {
    cout << "接收到退出信号，正在关闭客户端..." << endl;
    isrunning = false;
}
int main()
{
    signal(SIGINT, signalHandler);//Ctrl+C可以结束
    string username = "默认名称";
    string serverIP;
    int serverPort;

    cout << "请输入用户名：" << endl;
    getline(cin, username);
	if (username.empty()) {
		username = "默认名称";
	}

    cout << "请输入服务器IP地址(默认 127.0.0.1)" << endl;
    getline(cin, serverIP);
    if (serverIP.empty()) {
        serverIP = "127.0.0.1";
    }

    cout << "请输入服务器端口 (默认 8888): ";
    string portStr;
    getline(cin, portStr);
    if (portStr.empty()) {
        serverPort = 8888;
    }
    else {
        try {
            serverPort = stoi(portStr);
        }
        catch (...) {
            cerr << "无效的端口号，使用默认端口 8888" << endl;
            serverPort = 8888;
        }
    }
    TcpChatClient client(username, serverIP, serverPort);

    if (!client.start()) {
        cerr << "客户端程序启动失败" << endl;
        return 1;
    }
    cout << "已连接到聊天服务器,输入消息开始聊天，输入 exit 退出" << endl;

    string message;
    while (isrunning) {
        getline(cin, message);
        if (message == "exit") {
            break;
        }

        if (!message.empty()) {
            if (message.size() > MAX_MESSAGE_SIZE) {
                cout << "消息过长!" << endl;
                continue;
            }
            if (!client.SendMessage(message)) {
                cerr << "发送消息失败" << endl;
                break;
            }
        }
    }

    client.stop();
    cout << "客户端已关闭" << endl;
    return 0;
}

