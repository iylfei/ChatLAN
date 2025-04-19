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
		std::cerr << "Unknown command: " << cmd << std::endl;
		return false;
	}
}

bool CommandHandler::showUsersList()
{
	lock_guard<mutex> lock(client.mtx);
	// Send request to get online user list
	json messageJson;
	messageJson["type"] = "GetUserList";
	string jsonString = messageJson.dump();
	if (send(client.serverSocket, jsonString.c_str(), jsonString.size(), 0) < 0) {
		cerr << "Failed to get online user list: " << WSAGetLastError() << endl;
		return false;
	}

	return true;
}
