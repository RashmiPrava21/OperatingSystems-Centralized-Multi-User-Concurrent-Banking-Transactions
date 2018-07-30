#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <sstream>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

using namespace std;

struct Transaction {
        int t;
        int account_number;
        string trans_type;
        float amount;
};

int main (int argc, char* argv[])
{
	int clientSocket;
	int portNo;
	int n;
	int n1;
	int timeInSec = 0;
	int lineNum = 0;
	char buffer1[255];
	char msg[255];
	char buffer[1024];
   	std::string line, teime, val, amt; 
	struct sockaddr_in serverAddr;
	struct hostent *server;
	Transaction client_info;
    	ifstream clientFile("Transactions.txt");    
	bool ok = false;
	time_t start, end, diff;
	double time_spent, average_time;
	int counter = 0;	

	if(argc < 3)
    	{
        	cerr<<"Syntax : ./client <host name> <port>"<<endl;
        	return 0;
    	}
    
    	portNo = atoi(argv[2]);
    
    	if((portNo > 65535) || (portNo < 2000))
    	{
        	cerr<<"Please enter port number between 2000 - 65535"<<endl;
        	return 0;
    	}       
    
    	//create client socket
    	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    	if(clientSocket < 0)
    	{
        	cerr << "Cannot open socket" << endl;
        	return 0;
    	}
    
    	server = gethostbyname(argv[1]);
    
    	if(server == NULL)
    	{
        	cerr << "Host does not exist" << endl;
        	return 0;
    	}
    
    	memset((char *) &serverAddr, 0, sizeof(serverAddr));

    	serverAddr.sin_family = AF_INET;
    
    	bcopy((char *) server -> h_addr, (char *) &serverAddr.sin_addr.s_addr, server -> h_length);
    
    	serverAddr.sin_port = htons(portNo);
    
    	int checker = connect(clientSocket,(struct sockaddr *) &serverAddr, sizeof(serverAddr));
    
    	if (checker < 0)
    	{
        	cerr << "Cannot connect!" << endl;
        	return 0;
    	}
	else
	{
		memset(&buffer1, 0, sizeof(buffer1));//reads the message sent from server after successful connection
    		n = read(clientSocket, buffer1, 255);
    		if (n < 0) 
		{
         		cerr << "ERROR reading from socket" << endl;
		}
    		else
		{
			cout << buffer1 << endl << endl;
		}
	}

	//clock_t f;
	//f=clock();
	//cout << "Time in seconds: " << f/CLOCKS_PER_SEC;
    	cout << "List of accounts in the Transactions.txt:" << endl;
	
	//send transactions one by one to server
        while(getline(clientFile, line))
	{
		start = time (NULL);
		istringstream iss(line);
		lineNum++;
		bool ok = false;
		string message;

		do 
		{
			if(!getline(iss,teime,' ')) 
				break;
			client_info.t = strtol(teime.c_str(),NULL,10);
			

			if(!getline(iss,val,' ')) 
				break;
			client_info.account_number = strtol(val.c_str(),NULL,10);
		
			if(!getline(iss,client_info.trans_type,' ')) 
				break;
			//client_info.trans_type = strtol(type.c_str(),NULL,10);

			if(!getline(iss,amt,' ')) 
				break;
			//client_info.amount = strtof(amt.c_str(),NULL);
			client_info.amount = stof(amt, 0);
			ok = true;
			
		}while(false);
		
		if(ok)
		{
			
			timeInSec = client_info.t - timeInSec;
 			cout << "Timestamp:" << " " <<client_info.t << "," << " Account Number:"<< " " << client_info.account_number
                << "," << " Transaction Type:" << " " << client_info.trans_type << "," << " Amount:" << client_info.amount << endl;

			sleep(timeInSec);
			memset(&buffer, 0, sizeof(buffer));
			strcpy(buffer,line.c_str());
			//cout << buffer << endl;
        		write(clientSocket, buffer, strlen(buffer));//sends the transaction information to server

 			n = read(clientSocket, msg, 300);//reads the message sent by server on successful transaction
         		if(n < 0)
			{
                		cout << "Error reading message";
			}
       			cout << "\nMessage from server: " << msg << endl << endl; 
			
 			
			message = string(msg, n);
			//cout << message << endl;
			if(message.find("New Customer") != string::npos) //checks if the transaction details sent is not present in the Records.txt and a new account needs to be created
			{
				string data;
				getline(cin, data);
				memset(&msg, 0, sizeof(msg));
				strcpy(msg, data.c_str());
				write(clientSocket, msg, strlen(msg));//sends message whether the new account needs to be created or not
			}
		} 
		else 
		{
			cout << "Failed to parse line " << lineNum<< " :" << line << endl;
		}
		end= time(NULL);
		diff = difftime (end,start);
		counter = counter + 1;
		time_spent = time_spent + abs(diff);
		cout << "Time spent: " << diff << endl;
	}
	//f = clock() - f;
	//cout << "Time in seconds: " << f/CLOCKS_PER_SEC << endl<<endl;
	cout << "Total time: " << time_spent << endl;
	average_time = time_spent/counter;
	cout << "Average time: " << average_time << endl;
	cout <<"Successfully sent transaction details to server" << endl;
	clientFile.close();
}

/* References: 
   https://www.cprogramming.com/tutorial/string.html
   http://programmingknowledgeblog.blogspot.com/2013/05/c-program-to-display-current-date-and.html
   http://www.cplusplus.com/reference/sstream/istringstream/istringstream/
   https://solarianprogrammer.com/2012/02/27/cpp-11-thread-tutorial-part-2/
   http://www.bogotobogo.com/cplusplus/sockets_server_client.php
   https://stackoverflow.com/questions/1092631/get-current-time-in-seconds-since-the-epoch-on-linux-bash
   http://www.cplusplus.com/reference/ctime/time/
   https://solarianprogrammer.com/2011/12/16/cpp-11-thread-tutorial/
   https://www.justsoftwaresolutions.co.uk/threading/managing_threads_with_a_vector.html */
