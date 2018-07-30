#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <sstream>
#include <mutex>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <cmath>
#include <ctime>
#include <stdlib.h>
#include <stdio.h>

using namespace std;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

struct Customer {//structure for saving customer details present in Records.txt
	int account_number;
	string name;
	float balance_amount;
};

struct Transaction {//structure for saving each transaction details sent by clients
	int t;
	int account_number;
	string trans_type;
	float amount;
};

void task(int);
vector<Customer> getAllAccountDetails(); //function to read all the account details from Records.txt
bool checkAccountExistInCustomerList(Transaction client_info, vector<Customer> customerList);//function to check if the account is present in Records.txt
vector<Customer> updateBankAccountForCustomer(Transaction client_info, vector<Customer> customerList);//function to update the required accounts for each transaction
Customer addBankAccountForCustomer(Transaction client_info, vector<Customer> customerList);//function to update new account details to Records.txt
void addInterest(vector<Customer> updatedCustomerList); //function to add interest to the amount in Records.txt

static int connFd;

int main(int argc, char* argv[])
{
	int pId, portNo, listenFd;
	//socklen_t len; //store size of the address
	bool loop = false;
	struct sockaddr_in svrAdd, clntAdd;
	vector<thread> threads;
	
	pthread_t threadA[100];

	if (argc < 2)
	{
		cerr << "Syntax : ./server <port>" << endl;
		return 0;
	}

	portNo = atoi(argv[1]);

	if((portNo > 65535) || (portNo < 2000))
	{
		cerr << "Please enter a port number between 2000 - 65535" << endl;
		return 0;
	}

	//create socket
	listenFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(listenFd < 0)
	{
		cerr << "Cannot open socket" << endl;
		return 0;
	}

	memset((char*) &svrAdd,0, sizeof(svrAdd));

	svrAdd.sin_family = AF_INET;
	svrAdd.sin_addr.s_addr = INADDR_ANY;
	svrAdd.sin_port = htons(portNo);

	//bind socket
	if(bind(listenFd, (struct sockaddr *)&svrAdd, sizeof(svrAdd)) < 0)
	{
		cerr << "Cannot bind" << endl;
		return 0;
	}

	listen(listenFd, 20);

	// len = sizeof(clntAdd);

	int noThread = 0;

	while (noThread < 100)
	{
		//cout << "Waiting for a client to connect"<< endl;

		socklen_t len = sizeof(clntAdd);

		//this is where client connects and server will hang in this mode until client connects
		connFd = accept(listenFd, (struct sockaddr *)&clntAdd, &len);

		if (connFd < 0)
		{
		    cerr << "Cannot accept connection" << endl;
		    return 0;
		}
		else
		{
		    cout << "Connection received from client" << endl;
		    write(connFd, "Message from server - Successfully connected\n", 44);
		}

		threads.push_back(thread(&task, connFd));

		noThread++;
		}


		for(auto& th : threads)
		th.join(); 

}

void task (int connFd)
{
	Transaction client_info;
	vector<Customer> customerList;
	vector<Customer> updatedCustomerList;
	bool flag, trans_flag;
	char test[300];
	int n;
	bool ok = false;
	//clock_t begin, end;
	time_t start, end, diff;
    	double time_spent, interest_time, interest_amount;
	Customer customer;
	//FILE *fp;

    	//cout << "Thread No: " << pthread_self() << endl;
	{
	memset(test, 0, sizeof(test));

	while((	n = read(connFd, test, 300))>0)
	{
		 if(n < 0)
			cout << "Error reading message";

		std::istringstream iss(test);
		//cout << test;

		do 
		{

			string time;
			if(!getline(iss,time,' ')) break;
			client_info.t = strtol(time.c_str(),NULL,10);
			string val;
			if(!getline(iss,val,' ')) break;
			client_info.account_number = strtol(val.c_str(),NULL,10);
			//string type;
			if(!getline(iss,client_info.trans_type,' ')) break;
			//client_info.trans_type = strtol(type.c_str(),NULL,10);
			string amt;
			if(!getline(iss,amt,' ')) break;
			//client_info.amount = strtof(amt.c_str(),NULL);
			client_info.amount = stof(amt, 0);
			//cout << client_info.amount<<endl;

		}while(false);

	
		cout<< "\nData received from client s"<< endl;
	 	cout << "Timestamp:" << " " <<client_info.t << "," << " Account Number:"<< " " << client_info.account_number
		        << "," << " Transaction Type:" << " " << client_info.trans_type << "," << " Amount:" << client_info.amount << endl;

		//begin = clock();
		start = time (NULL);
		//cout << "\nStartTime for each Transaction in seconds: " << begin/CLOCKS_PER_SEC << endl;
		pthread_mutex_lock(&m);
	

		//get all the Customer record details
		customerList = getAllAccountDetails();

		//check for account exist in the customer record list or not
		flag = checkAccountExistInCustomerList(client_info, customerList);

		//cout << "The flag value is " << " : " << flag << endl;
		//ofstream outFile("Records.txt");

		//sleep time interval to check for mutex lock is working or not
		sleep(5);

		//update Bank Account details for a customer in customer record list
		if(flag) 
		{
		    	updatedCustomerList = updateBankAccountForCustomer(client_info, customerList);
			
		}
		else if( flag == 0 && client_info.trans_type == "w") 
		{
		    	cerr << "Account does not exist in the bank record list" << endl;
		}
		else 
		{
			customer = addBankAccountForCustomer(client_info, customerList);
			if(!(customer.name == "NO"))
				customerList.push_back(customer);
			updatedCustomerList = customerList;

		}
		
		write(connFd, "Transaction Complete\n", 20);//sends confirmation to client that the transaction is successful
		
		end= time(NULL);
 		diff = difftime (end,start);
		//end = (clock() - begin)/ CLOCKS_PER_SEC;
		//time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
		//printf("It took me (%f seconds).\n",((float)time_spent)/CLOCKS_PER_SEC);
		//cout << "\n Connection ID and Time taken for recent transaction " << endl;		
		cout << "Time taken: " << diff << endl;

		interest_time = abs(diff + interest_time);
		
		//cout << "Connection ID and Seconds after recent transaction:" << endl;
		//cout << connFd << " " << interest_time << endl;

		/*fp = fopen("Details.txt", "wb");
		if(fp == NULL)
    		{
			printf("Error opening file\n");
			exit(1);
   		}
		printf("Testing fwrite() function: \n\n");
		printf("%d \t %d", connFd, interest_time);
		fclose(fp);*/

		vector<Customer>::iterator ptr;
		if(interest_time >= 30)
		{
			addInterest(updatedCustomerList);
			interest_time = 0.00;
			/*cout << "Interest added after 30 seconds:" << endl;
			for(ptr = updatedCustomerList.begin(); ptr < updatedCustomerList.end(); ptr++)
			{	
				
				interest_amount = double((*ptr).balance_amount) * 0.05;
				(*ptr).balance_amount = (*ptr).balance_amount + float(interest_amount);
				//cout << "Interest added after 30 seconds:" << endl;
				cout << "Account Number:"<< " " << (*ptr).account_number << "," << " Account Name:" << " "
                                        << (*ptr).name << "," << " Balance Amount:" << " " << (*ptr).balance_amount << endl;
				outFile << (*ptr).account_number << " " << (*ptr).name << " " << (*ptr).balance_amount << "\n";
				//interest_time = 0.00;
			}
			interest_time = 0.00;*/
		}
		/*for(ptr = updatedCustomerList.begin(); ptr < updatedCustomerList.end(); ptr++)
		{
			outFile << (*ptr).account_number << " " << (*ptr).name << " " << (*ptr).balance_amount << "\n";
			//cout << "Updated Records.txt" << endl;
		}
		cout << "Successfully updated Records.txt" << endl;*/

		else
		{
			ofstream outFile("Records.txt");
			for(ptr = updatedCustomerList.begin(); ptr < updatedCustomerList.end(); ptr++)
			{
				outFile << (*ptr).account_number << " " << (*ptr).name << " " << (*ptr).balance_amount << "\n";
				//cout << "Updated Records.txt" << endl;
			}
			outFile.close();
		}
		//outFile.close();
		//write(connFd, "Transaction Complete\n", 20);

		pthread_mutex_unlock(&m);
		//end = clock();
		//cout << "Time taken by each transaction in seconds: " << begin/CLOCKS_PER_SEC << endl;
		//time_spent = (double)(end - begin) / CLOCKS_PER_SEC;//calculates the time taken to execute each transaction
		//cout << connFd << endl;
	    	//cout << "\nTime taken by each transaction: " << time_spent << endl;
		//Customer addInterest(time_spent, updatedCustomerList);

		memset(test, 0, sizeof(test));
	}}
	
	cout << "\nClosing thread and conn" << endl;
	close(connFd);

}


vector<Customer> getAllAccountDetails()
{
	ifstream inFile("Records.txt");
	vector<Customer> listOfCustomers;
	string line;
	int lineNum = 0;

	while(getline(inFile, line))
	{
		istringstream iss(line);
		lineNum++;

		// Try to extract account data from the line
		Customer c;
		bool ok = false;

		do {
			string id;
			if(!getline(iss, id, ' ')) 
				break;
			c.account_number = strtol(id.c_str(), NULL, 10);
		
			if(!getline(iss, c.name, ' '))
				break;
		
			string amt;
			if(!getline(iss, amt, ' '))
				break;
			//c.balance_amount = strtol(amt.c_str(), NULL, 10);
			c.balance_amount = stof(amt, 0);

			ok = true;
		}while(false);

		//After successfully extracting the detials, below statement updates the vector listofCustomers with those details
		if(ok) 
		{
			listOfCustomers.push_back(c);
		} 
		else 
		{
			cout << "Failed to parse line " << lineNum << " : " << line << endl;
		}
	}

	return listOfCustomers; 

}

bool checkAccountExistInCustomerList(Transaction client_info, vector<Customer> customerList)
{
	bool flag = false;
	vector<Customer>::iterator ptr;

	cout << "\nList of accounts in the Records.txt:" << '\n';

	for(ptr = customerList.begin(); ptr < customerList.end(); ptr++)//displays all the details present in Records.txt. Also, checks whether the account number of each transaction is present in the Records.txt
	{
		cout << "Account Number:"<< " " << (*ptr).account_number << "," << " Account Name:" << " "
                                        << (*ptr).name << "," << " Balance Amount:" << " " << (*ptr).balance_amount << endl;

		if(client_info.account_number == (*ptr).account_number) {
			flag = true;
		}
		

	}

	return flag;
}


vector<Customer> updateBankAccountForCustomer(Transaction client_info, vector<Customer> customerList)
{
	vector<Customer>::iterator ptr;

	for(ptr = customerList.begin(); ptr < customerList.end(); ptr++)
	{
		if(client_info.account_number == (*ptr).account_number)//traverses through the entire Records.txt, checks which account needs to be updated and performs the required operation
		{
			if(!client_info.trans_type.compare("w"))//checks if the transaction is for withdrawing money
			{
				(*ptr).balance_amount = (*ptr).balance_amount - client_info.amount;	
				if( (*ptr).balance_amount < 0) 
				{
					(*ptr).balance_amount = (*ptr).balance_amount + client_info.amount;	
					cerr << "\n \nFunds Not Available" << endl<< endl;
				}

			} 
			else if(!client_info.trans_type.compare("d"))//checks if the transaction is for depositing money
			{
				(*ptr).balance_amount = (*ptr).balance_amount + client_info.amount;	
			}
			else 
			{
				cerr << "Invalid Transaction Type" << '\n';
			}
			cout << "\nUpdated Record from the list:" << endl;
			cout << "Account Number:"<< " " << (*ptr).account_number << "," << " Account Name:" << " "
                                        << (*ptr).name << "," << " Balance Amount:" << " " << (*ptr).balance_amount << endl;
			
			//write(connFd, "Transaction Complete\n", 20);

		}
	}

	return customerList;
}

Customer addBankAccountForCustomer(Transaction client_info, vector<Customer> customerList)
{
	char msg[1500];
	int n;
	Customer cust;

	write(connFd,"Record not found. New Customer. If you want to add in list. Enter the name or else press enter\n", 255);//notifies client that the account number  mentioned in the transaction details is not present in the Record.txt
	memset(msg, 0, sizeof(msg));

	n = read(connFd, msg, 300);//checks whether a new record needs to be added in Records.txt

        if(n < 0)
                cout << "Error reading message";
	
	if(n == 0)
	{
		cust = {0, "dummy", 0};
	} 
	else //if client agrees to create a new account then it creates one with the transaction details provided
	{
		cust.account_number = client_info.account_number;
		cust.name = string(msg, n);
		cust.balance_amount = client_info.amount;

		cout << "\nNew Account need to be added in the Records.txt depending on client choice:" << endl;
		cout << "Account Number:"<< " " << cust.account_number << "," << " Account Name:" << " "
                                        << cust.name << "," << " Balance Amount:" << " " << cust.balance_amount << endl;
	}
	return cust;
}

void addInterest(vector<Customer> updatedCustomerList)
{
	double interest_amount;
	vector<Customer>::iterator ptr;
	cout << "Interest of 5 percent added after 30 seconds:" << endl;
	ofstream outFile("Records.txt");
	for(ptr = updatedCustomerList.begin(); ptr < updatedCustomerList.end(); ptr++)
	{	
		
		interest_amount = double((*ptr).balance_amount) * 0.05;
		(*ptr).balance_amount = (*ptr).balance_amount + float(interest_amount);
		//cout << "Interest added after 30 seconds:" << endl;
		cout << "Account Number:"<< " " << (*ptr).account_number << "," << " Account Name:" << " "
                        << (*ptr).name << "," << " Balance Amount:" << " " << (*ptr).balance_amount << endl;
		outFile << (*ptr).account_number << " " << (*ptr).name << " " << (*ptr).balance_amount << "\n";
		//interest_time = 0.00;
	}
	outFile.close();
	//interest_time = 0.00;
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
