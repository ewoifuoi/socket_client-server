#include<bits/stdc++.h>
#include<WinSock2.h>
#include<Windows.h>

using namespace std;

#define WSVERS MAKEWORD(2, 0)
#define LINELEN 128

void show(string text, int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
    cout << endl << text << endl << endl;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    
}

SOCKET passiveUDP(const char *service);
SOCKET passiveTCP(const char *service, int qlen);
SOCKET passivesock(const char *service, const char * transport, int qlen);


int main() {

    char buf[LINELEN + 1];

    struct sockaddr_in fsin;
    string service = "daytime";
    SOCKET sock;
    int alen;
    char * pts;
    time_t now;
    WSADATA wsadata;

    if(WSAStartup(WSVERS, &wsadata) != 0) {
        show("套接字初始化失败 !", 4);
    }

    sock = passiveUDP(service.data());

    while(1) {
        alen = sizeof(struct sockaddr);
        if(recvfrom(sock, buf, sizeof(buf), 0, 
        (struct sockaddr *)&fsin, &alen) == SOCKET_ERROR) {
            stringstream ss; ss << "recvfrom 接收数据失败: " << GetLastError();
            show(ss.str(), 4);
        }
            
        (void)time(&now);
        pts = ctime(&now);
        stringstream ss; ss << "服务器当前时间: " << pts;
        show(ss.str(), 5);
        (void) sendto(sock, pts, strlen(pts), 0,
        (struct sockaddr *)&fsin, sizeof(fsin));
    }

    return 1;
}

SOCKET passiveUDP(const char *service) {
    return passivesock(service, "udp", 0);
}

SOCKET passiveTCP(const char *service, int qlen) {
    return passivesock(service, "tcp", qlen);
}


SOCKET passivesock(const char *service, const char * transport, int qlen) {
    struct servent *pse;
    struct protoent *ppe;
    struct sockaddr_in sin;
    SOCKET s;
    int type;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY; // 服务端指定 IP 地址需要注意
    
    if(pse = getservbyname(service, transport)) {
        sin.sin_port = (u_short)pse->s_port;
        stringstream ss; ss << "服务端口解析成功: " << service << " -> " << pse->s_port;
        show(ss.str(), 3);
    }
    else if((sin.sin_port = htons((u_short)atoi(service))) == 0) {
        show("服务端口解析失败 !", 4);
    }

    if((ppe = getprotobyname(transport)) == 0) {
        show("无法解析传输层协议", 4);
    }
    if(strcmp(transport, "udp") == 0) {
        type = SOCK_DGRAM;
    }
    else {
        type = SOCK_STREAM;
    }

    s = socket(PF_INET, type, ppe->p_proto);

    if(s == INVALID_SOCKET) {
        stringstream ss; ss << "创建套接字失败: " << GetLastError();
        show(ss.str(), 4);
    }
    else {
        stringstream ss; ss << "套接字标识符: " << s;
        show(ss.str(), 3);
    }
    if(bind(s, (struct sockaddr *)&sin, sizeof(sin)) == SOCKET_ERROR) {
        stringstream ss; ss << "连接失败: " << GetLastError();
        show(ss.str(), 4); 
    }
    // 流式套接字需要调用 listen
    if(type == SOCK_STREAM && listen(s, qlen) == SOCKET_ERROR) {
        stringstream ss; ss << "监听失败: " << GetLastError();
        show(ss.str(), 4);
    }


    return s;
}