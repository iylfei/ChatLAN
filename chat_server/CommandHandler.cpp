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
		cout << "�������" << endl;
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
	cout << "| ����       | ����                        |" << endl;
	cout << "+------------+-----------------------------+" << endl;
	cout << "| help       | ��ʾ�˰�����Ϣ              |" << endl;
	cout << "| broadcast  | �㲥һ����Ϣ�����пͻ���    |" << endl;
	cout << "+------------+-----------------------------+" << endl;
	return true;
}

bool CommandHandler::broadcast()
{
	string message;
	cout << "������Ҫ�㲥����Ϣ��" << endl;
	getline(cin, message);
	string fullMessage = "������: " + message;
	server.BroadcastMessage(fullMessage);
	cout << fullMessage << endl;
	return true;
}
