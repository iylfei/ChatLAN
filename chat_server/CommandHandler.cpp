#include "CommandHandler.hpp"
#include "chat_server.hpp"

CommandHandler::CommandHandler(TcpChatServer& server) : server(server)
{
	commands["/stop"] = [this]() { return stopServer(); };
	commands["/help"] = [this]() {return help(); };
	commands["/bc"] = [this]() { return broadcast(); };
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
	cout << "+------------+-----------------------------+" << endl;
	cout << "| 命令       | 描述                        |" << endl;
	cout << "+------------+-----------------------------+" << endl;
	cout << "| help       | 显示此帮助信息              |" << endl;
	cout << "| broadcast  | 广播一条消息给所有客户端    |" << endl;
	cout << "+------------+-----------------------------+" << endl;
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
