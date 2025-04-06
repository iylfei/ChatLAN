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

void TcpChatServer::HandleClient(SOCKET clientSocket)
{
    //�洢�ͻ�������
    char clientsName[32];
	memset(clientsName, 0, sizeof(clientsName));
	int receivedBytes = recv(clientSocket, clientsName, sizeof(clientsName) - 1, 0);//��һ�ֽڸ�������
    if (receivedBytes > 0) {
		string clientName(clientsName);
		{
			lock_guard<mutex> lock(clientsMutex);
			clientNames[clientSocket] = clientName;
		}
		cout << clientName << " �Ѽ�������" << endl;
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
				auto it = clientNames.find(clientSocket);
				if (it != clientNames.end()) {
                    username = it->second;
					exitMessage = username + " ���˳�����";
					clientNames.erase(it);
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

