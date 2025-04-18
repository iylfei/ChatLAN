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
	string message = "getUsers";
	{
		lock_guard<mutex> lock(mtx);
		// ���ͻ�ȡ�����û��б������
		if (send(client.serverSocket, message.c_str(), message.size(), 0) < 0) {
			cerr << "��ȡ�����û��б�ʧ��" << WSAGetLastError() << endl;
			return false;
		}
		// ���������û��б�
		char buffer[MAX_MESSAGE_SIZE];
		memset(buffer, 0, sizeof(buffer));
		int bytesReceived = recv(client.serverSocket, buffer, sizeof(buffer) - 1, 0);
		if (bytesReceived <= 0) {
			cerr << "���������û��б�ʧ��" << WSAGetLastError() << endl;
			return false;
		}
		buffer[bytesReceived] = '\0'; // ȷ���ַ�������
		string userList(buffer);
		cout << "�����û��б�:" << endl;

	}
	return true;
}

void CommandHandler::handleJsonMessage(const json& jsonMsg)
{
	
}

