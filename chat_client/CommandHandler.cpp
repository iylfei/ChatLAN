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
	string message = "getUsers";
	{
		lock_guard<mutex> lock(mtx);
		// 发送获取在线用户列表的请求
		if (send(client.serverSocket, message.c_str(), message.size(), 0) < 0) {
			cerr << "获取在线用户列表失败" << WSAGetLastError() << endl;
			return false;
		}
		// 接收在线用户列表
		char buffer[MAX_MESSAGE_SIZE];
		memset(buffer, 0, sizeof(buffer));
		int bytesReceived = recv(client.serverSocket, buffer, sizeof(buffer) - 1, 0);
		if (bytesReceived <= 0) {
			cerr << "接收在线用户列表失败" << WSAGetLastError() << endl;
			return false;
		}
		buffer[bytesReceived] = '\0'; // 确保字符串结束
		string userList(buffer);
		cout << "在线用户列表:" << endl;

	}
	return true;
}

void CommandHandler::handleJsonMessage(const json& jsonMsg)
{
	
}

