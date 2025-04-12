#include "CommandHandler.hpp"
#include "chat_server.hpp"

CommandHandler::CommandHandler(TcpChatServer& server) : server(server)
{
	commands["/stop"] = [this]() { return stopServer(); };
	commands["/help"] = [this]() {return help(); };
	commands["/bc"] = [this]() { return broadcast(); };
	commands["/cl"] = [this]() { return showClientList(); };
	commands["/dm"] = [this]() { return directMessage(); };
}

bool CommandHandler::handleCommand(const std::string& cmd)
{
	auto it = commands.find(cmd);
	if (it != commands.end())
	{
		return it->second();
	}
	else {
		cout << "命令不存在" << endl;
	}
	return true;
}

bool CommandHandler::stopServer()
{
	return false;
}


bool CommandHandler::help()
{
	cout << "命令" << endl;
	cout << "-----------------------------" << endl;
	cout << "/stop：关闭服务器" << endl;
	cout << "/dm：向客户端发起私聊" << endl;
	cout << "/bc：广播一条消息给所有客户端" << endl;
	cout << "/cl：显示聊天室当前在线列表" << endl;
	cout << "-----------------------------" << endl;
	cout << endl;
	return true;
}


bool CommandHandler::broadcast()
{
	string message;
	cout << "请输入要广播的消息：" << endl;
	getline(cin, message);
	string fullMessage = "服务器: " + message;
	server.BroadcastMessage(fullMessage);
	cout << fullMessage << endl;
	return true;
}

bool CommandHandler::showClientList()
{
	int count = 1;
	if (server.clientNames.empty()) {
		cout << "当前没有在线用户" << endl;
		return true;
	}
	for (const auto& client : server.clientNames) {
		cout << count << ". " << client.first << endl;
		count++;
	}
	return true;
}

//服务器私聊
bool CommandHandler::directMessage()
{
	string clientName;
	cout << "请输入要私聊的用户名：" << endl;
	getline(cin, clientName);
	if (server.clientNames.find(clientName) == server.clientNames.end()) {
		cout << "用户不存在" << endl;
		return false;
	}
	else {
		string message;
		cout << "请输入要发送的消息：" << endl;
		cout << "输入 /stopdm 结束私聊" << endl;
		while (true) {
			getline(cin, message);
			if (message == "/stopdm") {
				break;
			}
			string fullMessage = "服务器(私聊): " + message;
			SOCKET clientSocket = server.clientNames[clientName];
			if (send(clientSocket, fullMessage.c_str(), fullMessage.size(), 0) == SOCKET_ERROR)
			{
				cerr << "发送消息失败: " << WSAGetLastError() << endl;
				return false;
			}
		}
	}
	return true;
}