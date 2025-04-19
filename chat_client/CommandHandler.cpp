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
	lock_guard<mutex> lock(client.mtx); 
	// 发送获取在线用户列表的请求
	json messageJson;
	messageJson["type"] = "GetUserList";
	string jsonString = messageJson.dump();
	if (send(client.serverSocket, jsonString.c_str(), jsonString.size(), 0) < 0) {
		cerr << "获取在线用户列表失败" << WSAGetLastError() << endl;
		return false;
	}

	return true;
}
