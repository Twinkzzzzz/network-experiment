#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <iostream>
#include <string>
#pragma comment(lib, "Ws2_32.lib")
using namespace std;

#define serverPort 12345
#define serverIP "127.0.0.1"
#define myIP "127.0.0.5"
const string myIPstr="127.0.0.5|#|";

int main()
{
	char send_data[512],recvd_data[512];
	string tmp_string;
	
    // start
    WSADATA wsaData;
    int flag;
    flag=WSAStartup(MAKEWORD(2,2),&wsaData);
    if (flag!=0)
	{
        cout << "WSAStartup failed: %d\n" << flag;
        return 0;
    }
    
    // create socket
    SOCKET serversocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    //SOCKET serversocket=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    
    // bind serversocket
    SOCKADDR_IN addr;
    ZeroMemory(&addr,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=inet_addr(serverIP);
    addr.sin_port=htons(serverPort);
    bind(serversocket,(SOCKADDR*)&addr,sizeof(addr));
    
    //connect
    connect(serversocket,(SOCKADDR*)&addr,sizeof(addr));
    
    memset(send_data,'\0',sizeof(send_data));
    strcpy(send_data,myIP);
    send(serversocket,send_data,(int)strlen(send_data),0);
    
    //communicate
    memset(recvd_data,'\0',sizeof(recvd_data));
    recv(serversocket,recvd_data,512,0);
    cout << "[server] " << recvd_data << endl;
    
    while(1)
    {
    	getline(cin,tmp_string);
    	tmp_string=myIPstr+tmp_string;
    	
    	memset(send_data,'\0',sizeof(send_data));
    	tmp_string.copy(send_data,tmp_string.length()+1);
    	send(serversocket,send_data,(int)strlen(send_data),0);
    	if(tmp_string==myIPstr+"quit") break;
    	memset(recvd_data,'\0',sizeof(recvd_data));
    	recv(serversocket,recvd_data,512,0);
    	cout << "[server] " << recvd_data << endl;	
	}
    
    //disconnect
    closesocket(serversocket);
    WSACleanup();
    return 0;
    
}
