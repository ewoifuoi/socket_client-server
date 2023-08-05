#include<bits/stdc++.h>
#include<WinSock2.h>
#include<Windows.h>

using namespace std;

#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif
#define LINELEN 128
#define WSVERS MAKEWORD(2, 0)

#define MSG "what daytime is it\n"

SOCKET connectsock(const char *host, const char*service, const char*transport);
SOCKET connectTCP(const char *host, const char *service);
SOCKET connectUDP(const char *host, const char *service);

void TCPdaytime(const char *host, const char *service);
void UDPdaytime(const char *host, const char *service);

void show(string text, int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
    cout << endl << text << endl << endl;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
}

int main() {
    string host = "localhost";
    string service = "daytime";
    WSADATA wsadata;

    if(WSAStartup(WSVERS, &wsadata) != 0) {
        show("socket 初始化失败 !", 4);
    }
    TCPdaytime(host.data(), service.data());
    UDPdaytime(host.data(), service.data());
    WSACleanup();
    return 0;
}

void TCPdaytime(const char *host, const char *service) {
    char buf[LINELEN + 1];
    SOCKET s;
    int cc;
    s = connectTCP(host, service);

    cc = recv(s, buf, LINELEN, 0);

    while(cc != SOCKET_ERROR && cc > 0) {
        buf[cc] = '\0';
        (void) fputs(buf, stdout);
        cc = recv(s, buf, LINELEN, 0);
    }
    closesocket(s);
}

void UDPdaytime(const char *host, const char *service) {
    char buf[LINELEN+1];
    SOCKET s;
    int n;
    s = connectUDP(host, service);
    (void) send(s, MSG, strlen(MSG), 0);
    n = recv(s, buf, LINELEN, 0);
    if(n == SOCKET_ERROR) {
        stringstream ss; ss << "接收数据失败 :" << GetLastError();
        show(ss.str(), 4);
    }
    else {
        buf[n] = '\0';
        (void) fputs(buf, stdout);
    }
    closesocket(s);
}

SOCKET connectTCP(const char *host, const char *service) {
    return connectsock(host, service, "tcp");
}

SOCKET connectUDP(const char *host, const char *service) {
    return connectsock(host, service, "udp");
}

SOCKET connectsock(const char *host, const char*service, const char*transport) {
    struct hostent *phe;
    struct servent *pse;
    struct protoent *ppe;
    struct sockaddr_in sin;
    int s, type;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;

    if(pse = getservbyname(service, transport)) {
        sin.sin_port = pse->s_port;
        stringstream ss; ss << "服务端口解析成功: " << service << " -> " << pse->s_port;
        show(ss.str(), 3);
    }
    else if((sin.sin_port = htons((u_short)atoi(service))) == 0) {
        stringstream ss;ss << "服务解析失败 !" ;
        show(ss.str(), 4);
    }

    if(phe = gethostbyname(host)) {
        memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
        stringstream ss;ss << "域名到 IP 地址映射成功: " << host << " -> " <<inet_ntoa(*((struct in_addr *)phe->h_addr)) ;
        show(ss.str(), 3);
    }
    else if((sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE) {
        show("目标主机解析失败",4);
    }

    if((ppe = getprotobyname(transport)) == 0) {
        show("传输层协议有误!",4);
    }

    if(strcmp(transport, "udp") == 0) {
        type = SOCK_DGRAM;
    }
    else {
        type = SOCK_STREAM;
    }
    
    s = socket(PF_INET, type, ppe->p_proto);

    if(s == INVALID_SOCKET) {
        stringstream ss;ss << "创建套接字失败: " << GetLastError();
        show(ss.str(), 4);
    }
    if(connect(s, (struct sockaddr *) &sin, sizeof(sin)) == SOCKET_ERROR) {
        stringstream ss; ss << "连接失败: " << GetLastError();
        show(ss.str(), 4);
    }
    return s;
}
