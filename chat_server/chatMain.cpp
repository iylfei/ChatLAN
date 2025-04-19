#include "chat_server.hpp"
#include "CommandHandler.hpp"

int main()
{
	#ifdef _WIN32
		SetConsoleOutputCP(CP_UTF8);
	#endif

	TcpChatServer server(8888);

	if (!server.start()) {
		cerr << "Server startup failed: " << WSAGetLastError() << endl;
		return 1;
	}

	cout << "Server started, type /stop to stop..." << endl;
	cout << "Type /help to show command list" << endl;
	atomic<bool> isRunning = true;
	
	CommandHandler handler(server);
	string cmd;
	while (isRunning) {
		getline(cin, cmd);
		if (!handler.handleCommand(cmd)) {
			isRunning = false;
			break;
		}
	}
	server.stop();
	cout << "Server has been stopped" << endl;

	return 0;
}
