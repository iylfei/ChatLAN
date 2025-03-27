#include "chat_server.hpp"

int main()
{
    TcpChatServer server(8888);

    if (!server.start()) {
        cerr << "服务器启动失败" << WSAGetLastError() << endl;
        return 1;
    }

    cout << "服务器已启动，输入stop停止..." << endl;
    atomic<bool> isRunning = true;
    string cmd;
    while (isRunning) {
        getline(cin,cmd);
        if (cmd == "stop") {
            isRunning = false;
            break;
        }
    }

    server.stop();
    cout << "服务器已停止" << endl;

    return 0;
}

