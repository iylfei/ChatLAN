#include "chat_client.hpp"

bool TcpChatClient::InitNetwork()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "��ʼ��ʧ��" << WSAGetLastError() << endl;
        return false;
    }
    return true;
}

SOCKET TcpChatClient::CreateSocket()
{
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        cerr << "����socketʧ��" << WSAGetLastError() << endl;
    }
    return sock;
}

bool TcpChatClient::Connect()
{
    if (isConnected) return true;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(serverPort);
    
    if (inet_pton(AF_INET, serverIP.c_str(), &addr.sin_addr) <= 0) {
        std::cerr << "��Ч��IP��ַ" << std::endl;
        return false;
    }

    if (connect(serverSocket, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        cerr << "���ӷ�����ʧ��" << WSAGetLastError() << endl;
        return false;
    }
    isConnected = true;
    cout << "�ɹ����ӵ�������" << "IP:" << serverIP << " Port:" << serverPort << endl;

    //�����û�����������
	if (send(serverSocket, username.c_str(), username.size(),0) < 0) {
		cerr << "�����쳣����������״��" << WSAGetLastError() << endl;
		isConnected = false;
		return false;
	}
    
    return true;
}

bool TcpChatClient::SendMessage(const string& message)
{
    string fullMessage = username + ": " + message;
    if (send(serverSocket, fullMessage.c_str(), fullMessage.size(), 0) == SOCKET_ERROR) {
        if (WSAGetLastError() != 10054) {
            cerr << "������Ϣʧ��" << WSAGetLastError() << endl;
        }
        isConnected = false;
        return false;
    }
    return true;
}

void TcpChatClient::RecvMessage()
{
    char buff[4096];
    while (isConnected) {
        memset(buff, 0, sizeof(buff));
        int receivedBytes = recv(serverSocket, buff, sizeof(buff) - 1, 0);
        if (!isConnected)break;
        if (receivedBytes > 0) {
            if (receivedBytes > MAX_MESSAGE_SIZE) {
                cerr << "������Ϣ���ȳ�������" << endl;
                continue;
            }
            cout << buff << endl;
        }
        else {
            if (receivedBytes == 0) {
                cout << "�������ر�������" << endl;
            }
            else if (WSAGetLastError() == 10054) {
                cout << "�������ѹر�" << endl;
            }
            else  {
                cerr << "��Ϣ���ܴ���" << WSAGetLastError() << endl;
            }
            break;
        }
    }
    isConnected = false;
}

//�ͻ���˽��
