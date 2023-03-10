#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#pragma comment(lib, "Ws2_32.lib")
using namespace std;

#define myPort 12345
#define myIP "127.0.0.1"

typedef struct ipstr
{
	int x[4];
}ipstr;

typedef struct message
{
	ipstr ip;
	char content[512];
}message;

typedef struct account
{
	ipstr ip;
	char password[16];
	bool online;
}account;

char recvd_data[512],send_data[512];
string tmp_string;
account acclist[128];
int accnum=0,flag; // logged in num
message tmp_message;

static int connect_num=0;
vector<SOCKET> client_list;

message get_message(char* recv)
{
	int tmp=0,k=0,i;
	ipstr ip;
	char c,content[512];
	for(i=0;i<(int)strlen(recv);++i)
	{
		if(recv[i]=='|' && recv[i+1]=='#' && recv[i+2]=='|') break;
		c=recv[i];
		if(c=='.')
		{
			ip.x[k]=tmp;
			k++;
			tmp=0;
		}
		else
		{
			tmp=tmp*10+(int)c-48;
		}
	}
	ip.x[k]=tmp;
	if(i+3<strlen(recv)) strcpy(content,recv+i+3); else memset(content,0,sizeof(content));
	message ans;
	ans.ip=ip;
	memset(ans.content,'\0',sizeof(ans.content));
	strcpy(ans.content,content);
	return ans;
}

void print_ip(ipstr ip)
{
	cout << "[" << ip.x[0] << "." << ip.x[1] << "." << ip.x[2] << "." << ip.x[3] << "]";
	return;
}

bool cmp_ip(ipstr ip1, ipstr ip2)
{
	if(ip1.x[0] != ip2.x[0]) return false;
	if(ip1.x[1] != ip2.x[1]) return false;
	if(ip1.x[2] != ip2.x[2]) return false;
	if(ip1.x[3] != ip2.x[3]) return false;
	return true;
}

int search_account(ipstr ip)
{
	int i;
	for(i=1;i<=accnum;++i)
	{
		if(cmp_ip(ip,acclist[i].ip))
		{
			break;
		}
	}
	if(!(i>=1 && i<=accnum)) i=0;
	return i;
}

DWORD WINAPI server_thread(LPVOID lp_thread_parameter);

int main()
{
    // start
    WSADATA wsaData;
    flag=WSAStartup(MAKEWORD(2,2),&wsaData);
    if (flag!=0)
	{
        cout << "WSAStartup failed: %d\n" << flag;
        return 0;
    }
    
    // create socket
    SOCKET serversocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    //SOCKET serversocket=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    
    if (serversocket==INVALID_SOCKET)
    {
        cout << "socket create failed!\n";
        return 0;
    }
    
    // bind socket
    SOCKADDR_IN addr;
    ZeroMemory(&addr,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=inet_addr(myIP);
    //addr.sin_addr.S_un.S_addr=inet_addr(myIP);
    addr.sin_port=htons(myPort);
    flag=bind(serversocket,(SOCKADDR*)&addr,sizeof(addr));
    if(flag==-1)
    {
    	cout << "bind failed!\n";
        return 0;
	}
    cout << "server ready......\n";
    
    unsigned long ul = 1;
    while(1)
    {
    	if (listen(serversocket,20)!=-1)
    	{
    		//cout << "waiting for connection......\n";
    		SOCKADDR clientaddr;
    		int size=sizeof(clientaddr);
    		SOCKET clientsocket=accept(serversocket,&clientaddr,&size);

            // search the account
            vector<SOCKET>::iterator itr = find(client_list.begin(),client_list.end(),clientsocket);
            if (itr==client_list.end()) client_list.push_back(clientsocket);

            DWORD dwPingThreadID;
            //创建新线程处理该客户socket连接
            HANDLE hPingHandle=CreateThread(0,0,server_thread,(LPVOID)clientsocket,0,&dwPingThreadID);
		}
	}
	for (int i=0;i<client_list.size();i++)
    {
        closesocket(client_list[i]);
    }
    client_list.clear();
    WSACleanup();
    return 0;
}

DWORD WINAPI server_thread(LPVOID lp_thread_parameter)
{
	ipstr ip_this;
	SOCKET clientsocket=(SOCKET)lp_thread_parameter;
	
	memset(recvd_data,'\0',sizeof(recvd_data));
    recv(clientsocket,recvd_data,512,0);
    tmp_message=get_message(recvd_data);
    cout << "receive a connection from ";
    print_ip(tmp_message.ip);
    connect_num++;
    cout << "   recent connection: " << connect_num << endl;
    ip_this=tmp_message.ip;
    
	memset(send_data,'\0',sizeof(send_data));
    strcpy(send_data,"server connected");
    send(clientsocket,send_data,(int)strlen(send_data),0);
    while(1)
    {
    	memset(recvd_data,'\0',sizeof(recvd_data));
    	flag=recv(clientsocket,recvd_data,512,0);
    	//cout << flag << endl;
    	if(flag==-1)
    	{
    		int i=search_account(ip_this);
    		if(i>0) acclist[i].online=false;
			closesocket(clientsocket);
			cout << "lose connection from ";
			print_ip(ip_this);
			connect_num--;
			cout << "   recent connection: " << connect_num << endl;
			vector<SOCKET>::iterator itr = find(client_list.begin(),client_list.end(),clientsocket);
			if(itr!=client_list.end()) client_list.erase(itr);
			break;
		}
    	//cout << recvd_data << endl;
    	tmp_message=get_message(recvd_data);
    	//if(!strcmp(tmp_message.content,"")) break;
    	print_ip(tmp_message.ip);
    	cout << " ";
    	cout << tmp_message.content << endl;
    	memset(send_data,'\0',sizeof(send_data));
    	int i=search_account(tmp_message.ip);
    	if(!strcmp(tmp_message.content,"quit"))
		{
			if(i>0) acclist[i].online=false;
			closesocket(clientsocket);
			cout << "lose connection form ";
			print_ip(tmp_message.ip);
			connect_num--;
			cout << "   recent connection: " << connect_num << endl;
			vector<SOCKET>::iterator itr = find(client_list.begin(),client_list.end(),clientsocket);
			if(itr!=client_list.end()) client_list.erase(itr);
			break;
		}
    	if(i && acclist[i].online)
    	{
    		if(!strcmp(tmp_message.content,"login"))
    		{
    			strcpy(send_data,"already logged in, do you mean 'logout'?");
			}
			else if(!strcmp(tmp_message.content,"logout"))
			{
				acclist[i].online=false;
				strcpy(send_data,"logged out successfully!");
			}
			else
			{
				strcpy(send_data,"message received!");
			}
		}
		else
		{
			if(!strcmp(tmp_message.content,"login"))
    		{
    			memset(recvd_data,'\0',sizeof(recvd_data));
    			if(i) //not first time, but now offline
    			{
    				strcpy(send_data,"password:");
    				send(clientsocket,send_data,(int)strlen(send_data),0);
    				memset(send_data,'\0',sizeof(send_data));
    				recv(clientsocket,recvd_data,512,0);
    				tmp_message=get_message(recvd_data);
    				print_ip(tmp_message.ip);
    				cout << " ";
    				cout << tmp_message.content << endl;
    				if(strcmp(tmp_message.content, acclist[i].password))
    				{
    					strcpy(send_data,"wrong password!");
					}
					else
					{
						acclist[i].online=true;
						strcpy(send_data,"logged in successfully!");
					}
				}
				else //first time
				{
					strcpy(send_data,"frist time log in,set password:");
    				send(clientsocket,send_data,(int)strlen(send_data),0);
    				memset(send_data,'\0',sizeof(send_data));
    				recv(clientsocket,recvd_data,512,0);
    				tmp_message=get_message(recvd_data);
    				print_ip(tmp_message.ip);
    				cout << " ";
    				cout << tmp_message.content << endl;
    				accnum++;
    				acclist[accnum].ip=tmp_message.ip;
    				memset(acclist[accnum].password,'\0',sizeof(acclist[accnum].password));
    				strcpy(acclist[accnum].password,tmp_message.content);
    				acclist[accnum].online=true;
    				strcpy(send_data,"logged in successfully!");
				}
			}
			else if(!strcmp(tmp_message.content,"logout"))
			{
				strcpy(send_data,"not online, do you mean 'login'?");
			}
			else
			{
				strcpy(send_data,"not online, type 'login' to log in");
			}
		}
    	send(clientsocket,send_data,(int)strlen(send_data),0);
    	Sleep(50);
	}
}
