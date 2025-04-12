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
		std::cerr << "未知命令: " << cmd << std::endl;
		return false;
	}
}

bool CommandHandler::showUsersList()
{
	std::cout << "在线用户列表:" << std::endl;

	return false;
}

