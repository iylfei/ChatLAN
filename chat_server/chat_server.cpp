#include "chat_server.hpp"

//��ʼ�������
bool TcpChatServer::InitNetwork()
{
	WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "��ʼ��Winsockʧ��" << WSAGetLastError() << endl;
        return false;
    }
    return true;
}

//�����׽���
SOCKET TcpChatServer::CreateSocket()
{
    SOCKET sock = socket(AF_INET,SOCK_STREAM,0);
    if (sock == INVALID_SOCKET) {
        cerr << "����socketʧ��" << WSAGetLastError() << endl;
    }
    return sock;
}

// ���õ�ַ
void TcpChatServer::ConfigSocketAddress(sockaddr_in& addr, int port) {
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
}

// �󶨶˿�
bool TcpChatServer::BindSocket() {
    sockaddr_in server_addr{};
    ConfigSocketAddress(server_addr, serverPort);

    if (bind(serverSocket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        cerr << "��ʧ��" << WSAGetLastError() << endl;
        return false;
    }
    return true;
}


//��ʼ����
bool TcpChatServer::StartListen()
{
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "����ʧ��" << WSAGetLastError() << endl;
        return false;
    }
    cout << "�����ɹ����ȴ�����..." << endl;
    return true;
}

//���տͻ�������
void TcpChatServer::AcceptClients()
{
    while (isRunning) {
        sockaddr_in client_addr{};  // ʹ��client_addr���洢�ͻ��˵�ַ
        int addrlen = sizeof(client_addr);

        SOCKET clientSocket = accept(serverSocket,(sockaddr*)&client_addr, &addrlen);
        if (clientSocket == INVALID_SOCKET) {
            if (isRunning) {
                cerr << "��������ʧ��: " << WSAGetLastError() << endl;
            }
            continue;
        }

        //������ӿͻ�����Ϣ
        //�洢ת�����IP��ַ�ַ���
        char client_ip[INET_ADDRSTRLEN];//INET_ADDRSTRLEN ��һ��Ԥ���峣������ʾ�洢IPv4��ַ�ַ������������ַ���
        //��������IP��ַת��Ϊ���ʮ���Ʊ�ʾ�����ַ���
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        cout << "�ͻ������ӳɹ���IP: " << client_ip << "���˿�: " << ntohs(client_addr.sin_port) << endl;

        //��ӿͻ������ӵ������б�
        //ʹ��������{}�����ֲ��������Ա�����ӵ��б�֮����������
        {
            lock_guard<mutex> lock(clientsMutex);
            clientSockets.push_back(clientSocket);
        }

        //�����½��̴���ͻ���
        thread clientThread(&TcpChatServer::HandleClient, this, clientSocket);
        clientThreads.push_back(move(clientThread));
    }
}

void TcpChatServer::HandleClient(SOCKET clientSocket) {
    bool isLogin = false;
    while (!isLogin) {
        try {
            char buffer[1024];
            memset(buffer, 0, sizeof(buffer));

            // ���տͻ�������
            int receivedBytes = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (receivedBytes <= 0) {
                if (receivedBytes == 0) {
                    cout << "�ͻ��������Ͽ�����" << endl;
                }
                else {
                    cerr << "���մ���: " << WSAGetLastError() << endl;
                }
                closesocket(clientSocket);
                return;
            }

            // ����JSON����
            json loginData;
            try {
                loginData = json::parse(buffer, buffer + receivedBytes);
            }
            catch (const json::parse_error& e) {
                json errorResponse;
                errorResponse["type"] = "InvalidFormat";
                errorResponse["message"] = "�Ƿ���JSON��ʽ";
                sendJsonMessage(errorResponse, clientSocket);
                continue;  
            }

            // ��֤�û�����ʽ
            if (!loginData.contains("username") ||
                !loginData["username"].is_string() ||
                loginData["username"].get<string>().empty()) {
                json errorResponse;
                errorResponse["type"] = "InvalidUsername";
                errorResponse["message"] = "�û�����ʽ���� (4-16λ��ĸ����)";
                sendJsonMessage(errorResponse, clientSocket);
                continue;
            }

            string username = loginData["username"];

            // ����û����ظ�
            {
                lock_guard<mutex> lock(clientsMutex);
                if (activeUsers.count(username)) {
                    json errorResponse;
                    errorResponse["type"] = "UsernameTaken";
                    errorResponse["message"] = "�û����ѱ�ռ�ã�����������";
                    sendJsonMessage(errorResponse, clientSocket);
                    continue;  
                }

                activeUsers.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(username),
                    std::forward_as_tuple(username, clientSocket)//ֱ�ӹ����������ο���
                );
            }

            // ��¼�ɹ�����
            json successResponse;
            successResponse["type"] = "LoginSuccess";
            successResponse["username"] = username;
            sendJsonMessage(successResponse, clientSocket);
            BroadcastMessage(username + " ������������",clientSocket);
            isLogin = true;  

        }
        catch (const exception& e) {
            cerr << "����ͻ����쳣: " << e.what() << endl;
            closesocket(clientSocket);
            return;  
        }
    }

    RecvMessage(clientSocket);
}


void TcpChatServer::RecvMessage(SOCKET clientSocket)
{
    char buff[4096];
    while (isRunning) {
        //��ջ�����
        memset(buff, 0, sizeof(buff));
        int receivedBytes = recv(clientSocket, buff, sizeof(buff) - 1, 0);//��һ�ֽڸ�������
        if (!isRunning)break;
        if (receivedBytes <= 0) {
			string exitMessage;
            string username;
			{
				lock_guard<mutex> lock(clientsMutex);
				for (const auto& client : clientNames) {
					if (client.second == clientSocket) {
						username = client.first;
						exitMessage = username + " ���˳�����";
						clientNames.erase(client.first);
						break;
					}
				}
			}
            BroadcastMessage(exitMessage, clientSocket);

            if (receivedBytes == 0) {
                cout << "�ͻ��� " << username << " �Ͽ�����" << endl;
            }
            else {
                int error = WSAGetLastError();
                if (error == 10053) {
                    cout << "�ͻ��� " << username << " �������������ֹ" << endl;
                }
                else if (error == 10054) {
                    cout << "�ͻ��� " << username << " �Ͽ�����" << endl;
                }
                else {
                    cerr << "������Ϣʧ��: " << error << endl;
                }
            }

            {
                lock_guard<mutex> lock(clientsMutex);
                auto pos = find(clientSockets.begin(), clientSockets.end(), clientSocket);
                if (pos != clientSockets.end()) {
                    clientSockets.erase(pos);
                }
            }
            closesocket(clientSocket);
            break;
        }
        string message(buff);
        cout <<  message << endl;
        BroadcastMessage(message, clientSocket);
    }
}

void TcpChatServer::SendMessage(SOCKET clientSocket, const string& message)
{
    if (send(clientSocket, message.c_str(), message.size(), 0) == SOCKET_ERROR) {
        cerr << "������Ϣʧ��: " << WSAGetLastError() << endl;
    }
}

void TcpChatServer::BroadcastMessage(const string& message, SOCKET excludeSocket)
{
    lock_guard<mutex> lock(clientsMutex);
    for (const auto& socket : clientSockets) {
        if (socket != excludeSocket) {
            SendMessage(socket, message);
        }
    }
}

//����JSON��Ϣ
bool TcpChatServer::sendJsonMessage(const json& jsonMsg,SOCKET clientSocket)
{
    string jsonString = jsonMsg.dump();
	lock_guard<mutex> lock(clientsMutex);
	if (send(clientSocket, jsonString.c_str(), jsonString.size(), 0) == SOCKET_ERROR)
	{
		cerr << "����JSON��Ϣʧ��: " << WSAGetLastError() << endl;
		return false;
	}
    return true;
}

//�����û��б�
bool TcpChatServer::sendUserList(SOCKET clientSocket)
{
    json userList;
	userList["type"] = "UserList";
    
}
