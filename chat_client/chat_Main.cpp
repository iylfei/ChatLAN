#include "chat_client.hpp"

atomic<bool> isrunning(true);

void signalHandler(int signal) {
    cout << "Received exit signal, closing client..." << endl;
    isrunning = false;
}
int main()
{
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
    #endif
    signal(SIGINT, signalHandler); // Ctrl+C to terminate
    string serverIP;
    int serverPort;

    cout << "Please enter server IP (default 127.0.0.1):" << endl;
    getline(cin, serverIP);
    if (serverIP.empty()) {
        serverIP = "127.0.0.1";
    }

    cout << "Please enter server port (default 8888): ";
    string portStr;
    getline(cin, portStr);
    if (portStr.empty()) {
        serverPort = 8888;
    }
    else {
        try {
            serverPort = stoi(portStr);
        }
        catch (...) {
            cerr << "Invalid port number, using default port 8888" << endl;
            serverPort = 8888;
        }
    }
    TcpChatClient client(serverIP, serverPort);

    if (!client.start()) {
        cerr << "Client startup failed" << endl;
        return 1;
    }
    cout << "Connected to chat server, type messages to chat, type /exit to quit" << endl;

    string message;
    while (isrunning) {
        getline(cin, message);
        if (message == "/exit") {
            break;
        }

        if (!message.empty()) {
            if (message.size() > MAX_MESSAGE_SIZE) {
                cout << "Message too long!" << endl;
                continue;
            }
            if (!client.SendChatMessage(message)) {
                cerr << "Failed to send message" << endl;
                break;
            }
        }
    }

    client.stop();
    cout << "Client has been closed" << endl;
    return 0;
}
