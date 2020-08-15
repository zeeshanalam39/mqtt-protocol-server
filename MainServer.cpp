#include <stdlib.h>
#include <iostream>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fstream>
#include <bits/stdc++.h>
#include <time.h>
#include <sstream>
using namespace std;

void *sendMsgThreadSub(void *);
void *recvMsgThreadSub(void *);
string convertToString(char *, int);
string ToString(char *);

#define BFRSIZE 100
string bfrStr = "", BFR = "";

struct Records
{
	int connfd = 0;
	string subId = "";
	bool sublight = false;
	bool subpressure = false;
	bool subproximity = false;
};
Records subrecords;
int main(void)
{
	// int noClients = 0;
	// // cout << "Enter number of Clients: ";
	// // cin >> noClients;

	// int noPubs = 0;
	// cout << "Enter number of pubs: ";
	// cin >> noPubs;
	// int noSubs = 0;
	// cout << "Enter number of subs: ";
	// cin >> noSubs;
	// noClients = noPubs + noSubs;

	// Create socket fd
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
	{
		cout << "Socket creation failed : " << errno;
		exit(-1);
	}
	else
	{
		cout << "Socket created successfully" << endl;
	}

	// Bind socket fd
	struct sockaddr_in addr;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(88);
	if (bind(fd, (sockaddr *)&addr, sizeof(addr)) == -1)
	{
		perror("Bind failed on socket : \n");
		exit(-1);
	}
	else
	{
		cout << "Bind success" << endl;
	}

	// Listen socket fd
	int backlog = 10;
	if (listen(fd, backlog) == -1)
	{
		perror("Listen failed on socket");
		exit(-1);
	}
	else
	{
		cout << "Listening...." << endl;
	}

	/* ---------- CREATE CONNFDs  ---------- */
	// Create socket connfd & accept clients
	int pubfd;
	struct sockaddr_in cliaddr;
	socklen_t cliaddr_len = sizeof(cliaddr);
	pubfd = accept(fd, (struct sockaddr *)&cliaddr, &cliaddr_len);
	if (pubfd == -1)
	{
		perror("accept failed on publisher socket ");
		exit(-1);
	}

	int connfd;
	struct sockaddr_in subaddr;
	socklen_t subaddr_len = sizeof(subaddr);
	// subrecords.connfd = accept(fd, (struct sockaddr *)&subaddr, &subaddr_len);
	connfd = accept(fd, (struct sockaddr *)&subaddr, &subaddr_len);
	subrecords.connfd = connfd;
	if (subrecords.connfd == -1)
	{
		perror("accept failed on socket connfd : ");
		exit(-1);
	}

	/* ---------- CREATE THREADS ---------- */
	// Send to sub/s thread/s
	int rc1;
	int rc2;
	pthread_t thdSendSub; // Thread
	rc1 = pthread_create(&thdSendSub, NULL, sendMsgThreadSub, (void *)connfd);
	if (rc1)
	{
		cout << "Error:unable to create thread," << rc1 << endl;
		//exit(-1);
	}
	pthread_t thdRecvFromSub;
	rc2 = pthread_create(&thdRecvFromSub, NULL, recvMsgThreadSub, (void *)connfd);
	if (rc2)
	{
		cout << "Error:unable to create thread," << rc2 << endl;
		//exit(-1);
	}

	/* ---------- RECEIVE READINGS FROM PUB/s ---------- */

	while (1)
	{
		char buffer[BFRSIZE] = "\0";
		string pressure = "", light = "", proximity = "";
		// memset(buffer, 0, sizeof(buffer));
		// Receive on socket connfd[0]
		recv(pubfd, buffer, sizeof(buffer), 0);
		cout << "Message Received from pub : " << buffer << "	Size: " << sizeof(buffer) << endl;
		bfrStr = ToString(buffer);
		cout << "ToString : " << bfrStr << "	Size: " << sizeof(bfrStr) << endl;
		BFR = bfrStr;
		// buffer = "\0";
		sleep(2);
	}
	close(fd);
	close(pubfd);
	//close(subrecords.connfd);
	//delete[] subrecords;
	//delete[] thdSendSub;
	//delete[] thdRecvFromSub;
	//delete[] subrecords;
	return 0;
}

// Convert char array to string
string convertToString(char *a, int size)
{
	int i;
	string s = "";
	for (i = 0; i < size; i++)
	{
		s = s + a[i];
	}
	return s;
}
string ToString(char *a)
{
	int i = 0;
	string s = "";
	while (a[i] != '\0')
	{
		s = s + a[i];
		i++;
	}
	return s;
}
// send to sub function
void *sendMsgThreadSub(void *sub)
{ // Thread function

	// struct Records *subrecord;
	// subrecord = (struct Records *)sub;

	long fd = (long)sub;
	while (true)
	{
		// char buffer[BFRSIZE] = "\0";
		string toSend;
		int i = 0;
		stringstream ss(bfrStr);
		vector<string> result;

		while (ss.good())
		{
			string substr;
			getline(ss, substr, ',');
			result.push_back(substr);
		}
		for (size_t i = 0; i < result.size(); i++)
		{
			cout << result[i] << "  Size: " << sizeof(result[i]) << endl;
		}

		if (result.size() >= 3)
		{
			if ((subrecords.subpressure == true) && (subrecords.sublight == true) && (subrecords.subproximity == true))
			{
				toSend = result[0] + "," + result[1] + "," + result[2];
			}

			else if ((subrecords.subpressure == true) && (subrecords.sublight == true))
				toSend = result[0] + "," + result[1];
			else if ((subrecords.subpressure == true) && (subrecords.subproximity == true))
				toSend = result[0] + "," + result[2];
			else if ((subrecords.subproximity == true) && (subrecords.sublight == true))
				toSend = result[1] + "," + result[2];
			else if (subrecords.subpressure == true)
				toSend = result[0];
			else if (subrecords.sublight == true)
				toSend = result[1];
			else if (subrecords.subproximity == true)
				toSend = result[2];
			else
				toSend = "";
			int size = toSend.length();
			char buffer[size + 1] = "\0";
			strcpy(buffer, toSend.c_str());
			bfrStr = "";
			// cout << "\nSent: " << buffer << endl;
			if ((send(fd, buffer, sizeof(buffer), 0)) > 0)
			{
				cout << "SENT: " << buffer << endl;
			}
			// memset(buffer, 0, sizeof(buffer));
			// buffer = "\0";
			//delete buffer;
		}
		sleep(2);
	}
}

// recv from sub
void *recvMsgThreadSub(void *sub)
{ // Thread function
	// struct Records *subrecord;
	// subrecord = (struct Records *)sub;
	long fd = (long)sub;
	while (true)
	{
		char buffer[100] = "\0";
		if (recv(fd, buffer, 100, 0) > 0)
		{
			// cout << "\n>>Message from subscriber: " << buffer << endl;
			string str = ToString(buffer);
			cout << "\n>>Message from subscriber: " << str << endl;
			if (str == "sublight")
			{
				subrecords.sublight = true;
				cout << " Subscribing Light..." << endl;
			}

			else if (str == "subpressure")
			{
				subrecords.subpressure = true;
				cout << " Subscribing Pressure..." << endl;
			}

			else if (str == "subproximity")
			{
				subrecords.subproximity = true;
				cout << " Subscribing Proximity..." << endl;
			}

			else if (str == "unsubproximity")
			{
				subrecords.subproximity = false;
				cout << " Unsubscribing Proximity..." << endl;
			}

			else if (str == "unsubpressure")
			{
				subrecords.subpressure = false;
				cout << " Unsubscribing Pressure..." << endl;
			}

			else if (str == "unsublight")
			{
				subrecords.sublight = false;
				cout << " Unsubscribing Light..." << endl;
			}

			else
				subrecords.subId = str;
			//memset(buffer, 0, sizeof(buffer));
		}
		sleep(2);
	}
}
