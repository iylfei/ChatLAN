#ifndef COMMAND_HANDLER_HPP
#define COMMAND_HANDLER_HPP

#include "chat_client.hpp"
#include <map>
#include <functional>

class CommandHandler {
public:
    CommandHandler(TcpChatClient& client);
    bool handleCommand(const std::string& cmd);

private:
    bool showUsersList();
    void handleJsonMessage(const json& jsonMsg);
    TcpChatClient& client;
    map<std::string, function<bool()>> commands;
};

#endif // !COMMAND_HANDLER_HPP




