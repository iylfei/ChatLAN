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
		cout << "Command does not exist" << endl;
	}
	return true;
}

bool CommandHandler::stopServer()
{
	return false;
}


bool CommandHandler::help()
{
	cout << "Commands" << endl;
	cout << "-----------------------------" << endl;
	cout << "/stop: Shutdown server" << endl;
	cout << "/dm: Send private message to a client" << endl;
	cout << "/bc: Broadcast a message to all clients" << endl;
	cout << "/cl: Show current online users in chat room" << endl;
	cout << "-----------------------------" << endl;
	cout << endl;
	return true;
}


bool CommandHandler::broadcast()
{
	string message;
	cout << "Please enter the message to broadcast:" << endl;
	getline(cin, message);
	string fullMessage = "Server: " + message;
	json broadcastMsg;
	broadcastMsg["type"] = "announcement";
	broadcastMsg["message"] = fullMessage;
	server.BroadcastMessage(broadcastMsg);
	cout << fullMessage << endl;
	return true;
}

bool CommandHandler::showClientList()
{
	int count = 1;
	if (server.activeUsers.empty()) {
		cout << "No users currently online" << endl;
		return true;
	}
	for (const auto& client : server.activeUsers) {
		cout << count << ". " << client.first << endl;
		count++;
	}
	return true;
}

// Server private messaging
bool CommandHandler::directMessage()
{
	string clientName;
	cout << "Please enter the username to direct message:" << endl;
	getline(cin, clientName);
	if (server.activeUsers.find(clientName) == server.activeUsers.end()) {
		cout << "User does not exist" << endl;
		return false;
	}
	else {
		string message;
		cout << "Please enter the message to send:" << endl;
		cout << "Enter /stopdm to end private messaging" << endl;
		while (true) {
			getline(cin, message);
			if (message == "/stopdm") {
				break;
			}
			string fullMessage = "Server(Private): " + message;
			SOCKET clientSocket = server.activeUsers[clientName].getSocket();
			json dmMsg;
			dmMsg["type"] = "dm";
			dmMsg["message"] = fullMessage;
			server.sendJsonMessage(dmMsg, clientSocket);
		}
	}
	return true;
}
