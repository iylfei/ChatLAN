#ifndef COMMAND_HANDLER_HPP
#define COMMAND_HANDLER_HPP

#include "chat_server.hpp"
#include <functional>
#include <map>


class CommandHandler {
public:
    CommandHandler(TcpChatServer& server);
    bool handleCommand(const std::string& cmd);
  
private:
	bool stopServer();
	bool help();
	bool broadcast();
    bool showClientList();
	bool directMessage();
    TcpChatServer& server;
    map<std::string, function<bool()>> commands;
};

#endif // COMMAND_HANDLER_HPP
