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
	cout << "����" << endl;
	cout << "-----------------------------" << endl;
	cout << "/stop���رշ�����" << endl;
	cout << "/dm����ͻ��˷���˽��" << endl;
	cout << "/bc���㲥һ����Ϣ�����пͻ���" << endl;
	cout << "/cl����ʾ�����ҵ�ǰ�����б�" << endl;
	cout << "-----------------------------" << endl;
	cout << endl;
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

bool CommandHandler::showClientList()
{
	int count = 1;
	if (server.clientNames.empty()) {
		cout << "��ǰû�������û�" << endl;
		return true;
	}
	for (const auto& client : server.clientNames) {
		cout << count << ". " << client.first << endl;
		count++;
	}
	return true;
}

//������˽��
bool CommandHandler::directMessage()
{
	string clientName;
	cout << "������Ҫ˽�ĵ��û�����" << endl;
	getline(cin, clientName);
	if (server.clientNames.find(clientName) == server.clientNames.end()) {
		cout << "�û�������" << endl;
		return false;
	}
	else {
		string message;
		cout << "������Ҫ���͵���Ϣ��" << endl;
		cout << "���� /stopdm ����˽��" << endl;
		while (true) {
			getline(cin, message);
			if (message == "/stopdm") {
				break;
			}
			string fullMessage = "������(˽��): " + message;
			SOCKET clientSocket = server.clientNames[clientName];
			if (send(clientSocket, fullMessage.c_str(), fullMessage.size(), 0) == SOCKET_ERROR)
			{
				cerr << "������Ϣʧ��: " << WSAGetLastError() << endl;
				return false;
			}
		}
	}
	return true;
}