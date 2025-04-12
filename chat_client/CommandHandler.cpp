#include "CommandHandler.hpp"

CommandHandler::CommandHandler(TcpChatClient& client) : client(client)
{
	commands["/users"] = [this]() { return showUsersList(); };
}

bool CommandHandler::handleCommand(const std::string& cmd)
{
	auto it = commands.find(cmd);
	if (it != commands.end()) {
		return it->second();
	}
	else {
		std::cerr << "δ֪����: " << cmd << std::endl;
		return false;
	}
}

bool CommandHandler::showUsersList()
{
	std::cout << "�����û��б�:" << std::endl;

	return false;
}

