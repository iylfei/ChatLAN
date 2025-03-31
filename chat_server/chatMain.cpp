#include "chat_server.hpp"
#include "CommandHandler.hpp"

int main()
{
    TcpChatServer server(8888);

    if (!server.start()) {
        cerr << "服务器启动失败" << WSAGetLastError() << endl;
        return 1;
    }

    cout << "服务器已启动，输入 /stop 停止..." << endl;
    cout << "输入 /help 查看命令列表" << endl;
    atomic<bool> isRunning = true;

	CommandHandler handler(server);
    string cmd;
    while (isRunning) {
        getline(cin, cmd);
        if (!handler.handleCommand(cmd)) {
            isRunning = false;
            break;
        }
    }
    server.stop();
    cout << "服务器已停止" << endl;

    return 0;
}

