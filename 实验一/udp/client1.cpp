#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <iostream>
#include <string>
#pragma comment(lib, "Ws2_32.lib")
using namespace std;

#define serverPort 12345
#define serverIP "127.0.0.1"
#define myIP "127.0.0.2"
const string myIPstr="127.0.0.2|#|";

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
    SOCKET clientsocket=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    
    SOCKADDR_IN addr;
    ZeroMemory(&addr,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.S_un.S_addr=inet_addr(serverIP);
    addr.sin_port=htons(serverPort);
    int server_len=sizeof(addr);
    
    //communicate
    while(1)
    {
    	getline(cin,tmp_string);
    	tmp_string=myIPstr+tmp_string;
    	if(tmp_string==myIPstr+"quit") break;
    	
    	memset(send_data,'\0',sizeof(send_data));
    	tmp_string.copy(send_data,tmp_string.length()+1);
    	
    	flag=sendto(clientsocket,send_data,sizeof(send_data),0,(SOCKADDR*)&addr,sizeof(addr));
		if (!flag)
		{
			cout << "fail to send!\n";
		}
    	
    	memset(recvd_data,'\0',sizeof(recvd_data));
    	flag=recvfrom(clientsocket,recvd_data,512,0,(SOCKADDR*)&addr,&server_len);
    	if(flag!=-1) cout << "[server] " << recvd_data << endl;	
	}
    
    //disconnect
    WSACleanup();
    return 0;
    
}
