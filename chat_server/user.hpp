#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

class user {
public:
	user() : clientSocket(INVALID_SOCKET), isOnline(false) {}
	user(const string& username, SOCKET clientSocket) : username(username), clientSocket(clientSocket),isOnline(true){}

	string getName() const { return username; }
	SOCKET getSocket() const { return clientSocket; }
	user* getUser() { return this; }
	void setOnline() { isOnline = true; }
private:
	string username;
	SOCKET clientSocket;
	bool isOnline;

};