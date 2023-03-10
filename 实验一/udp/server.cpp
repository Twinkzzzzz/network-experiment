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
	bool to_give_pw;
}account;

char recvd_data[512],send_data[512];
string tmp_string;
account acclist[128];
int accnum=0,flag; // logged in num
message tmp_message;
ipstr ip_this;

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
    SOCKET serversocket=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if (serversocket==INVALID_SOCKET)
    {
        cout << "socket create failed!\n";
        return 0;
    }
    
    // bind socket
    SOCKADDR_IN addr;
    ZeroMemory(&addr,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.S_un.S_addr=inet_addr(myIP);
    addr.sin_port=htons(myPort);
    flag=bind(serversocket,(SOCKADDR*)&addr,sizeof(addr));
    if(flag==-1)
    {
    	cout << "bind failed!\n";
        return 0;
	}
    cout << "server ready......\n";
    
	SOCKADDR_IN client_addr;
	memset(&client_addr,0,sizeof(client_addr));
	int client_len=sizeof(client_addr);
	SOCKET clientsocket;
	
	//communicate
    while(1)
    {
    	memset(recvd_data,'\0',sizeof(recvd_data));
    	recvfrom(serversocket,recvd_data,512,0,(SOCKADDR*)&client_addr, &client_len);
		bind(clientsocket,(SOCKADDR*)&client_addr,sizeof(client_addr));
		
    	tmp_message=get_message(recvd_data);
    	print_ip(tmp_message.ip);
    	cout << " ";
    	cout << tmp_message.content << endl;
    	memset(send_data,'\0',sizeof(send_data));
    	int i=search_account(tmp_message.ip);
    	//cout << i << endl;
    	if(!i)
    	{
    		if(!strcmp(tmp_message.content,"login"))
    		{
    			accnum++;
    			acclist[accnum].ip=tmp_message.ip;
    			memset(acclist[accnum].password,'\0',sizeof(acclist[accnum].password));
    			acclist[accnum].to_give_pw=true;
    			acclist[accnum].online=false;
    			strcpy(send_data,"frist time log in,set password:");
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
    	else if(i && acclist[i].online)
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
		else if(i && !acclist[i].online)
		{
			if(acclist[i].to_give_pw)
			{
				if(!strcmp(acclist[i].password,""))
				{
					memset(acclist[i].password,'\0',sizeof(acclist[i].password));
					strcpy(acclist[i].password,tmp_message.content);
					acclist[i].online=true;
					acclist[i].to_give_pw=false;
					strcpy(send_data,"logged in successfully!");
				}
				else
				{
					if(!strcmp(acclist[i].password,tmp_message.content))
					{
						acclist[i].online=true;
						acclist[i].to_give_pw=false;
						strcpy(send_data,"logged in successfully!");
					}
					else
					{
						acclist[i].online=false;
						acclist[i].to_give_pw=false;
						strcpy(send_data,"wrong password!");
					}
				}
			}
			else
			{
				if(!strcmp(tmp_message.content,"login"))
				{
					acclist[i].to_give_pw=true;
					strcpy(send_data,"password:");
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
		}
    	sendto(serversocket,send_data,sizeof(send_data),0,(SOCKADDR*)&client_addr,sizeof(client_addr));
    	Sleep(50);
	}
	return 0;
}

