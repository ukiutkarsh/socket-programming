#include <unistd.h>
#include<iostream>
#include<stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <vector>
#include <string>  
#include<sys/socket.h>
#include<arpa/inet.h>
#include <pthread.h>
using namespace std;

#include<dirent.h>
#include<fstream>
#include <chrono>
#include <thread>
#include<bits/stdc++.h>
#include<math.h>

int stringtoint(string str){
    const char *a = str.c_str();
    int l=str.length();int t=0;
    for(int i=0;i<l;i++) {
        t = t + pow(10,l-i-1)*(a[i]-48); //10*(a[0]-48)+(a[1]-48);
    }
    return(t);
}


int clientno;
int clientport;
int clientid;
int neighbourno;

void sending(int clientid, int *neighbourport, int neighbourno,int clientno,int clientport,string files);
void sending_back(int clientid,int clientno,int clientport,int neighbourport);
void receiving(int server_fd);
void *receive_thread(void *server_fd);
int PORT;

vector<int> received_data;
vector<string> received_files;

unordered_map<char, int> h;

bool compare(string x, string y)
{
	for (int i = 0; i < min(x.size(), y.size()); i++) {
		if (h[x[i]] == h[y[i]])
			continue;
		return h[x[i]] < h[y[i]];
	}
	return x.size() < y.size();
}

int main(int argc , char *argv[])
{
    
    string filelist[4096];int j=0; vector<string> v;
    DIR *d;
    struct dirent *dir;
    d = opendir(argv[2]);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
        	  v.push_back(dir->d_name);
            j++;
        }
        closedir(d);
    }

    string str = "._0123456789aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ";

	// Store the order for each character in the new alphabetical sequence
	h.clear();
	for (int i = 0; i < str.size(); i++)
		h[str[i]] = i;

	sort(v.begin(), v.end(), compare);

	// Print the strings after sorting
	string files=to_string(v.size()-2)+" ";
	for (auto x : v){
		if(x!="."&&x!=".."&&x!="..."&&x!="...."&&x!="....."&&x!="......"&&x!="......."&&x!="........"&&x!="........."&&x!=".........."&&x!="..........."){
            cout << x <<endl;
            files=files+x+" ";
           }
      }


    //reading file clienti-config.txt
    fstream file;
    string word, filename, A[4096];
  
    filename = argv[1];
  
    file.open(filename.c_str());
  
    int i=0;
    while (file >> word)
    {
        A[i] = word;
        i++;
    }
    clientno = stoi(A[0]);
    clientport = stoi(A[1]);
    clientid = stoi(A[2]);
    neighbourno = stoi(A[3]);
    int neighbours[neighbourno][2], neighbourport[neighbourno];
    for(int i=0; i < neighbourno; i++)
    {
        neighbours[i][0] = stoi(A[2*i+4]);   //storing the neighbours client number
        neighbours[i][1] = stoi(A[2*i+5]);   //storing the neighbours client port number
        neighbourport[i] = stoi(A[2*i+5]);
    }
    int filesearchno = stoi(A[2*neighbourno + 4]);
    string searchfile[filesearchno];
    for(int i=0; i < filesearchno; i++)
    {
        searchfile[i] = A[2*neighbourno + i+5];   //storing the searching file name
    }
    //sorting file search file for phase 2
    sort(searchfile, searchfile+filesearchno, compare);

    //establising the socket connection
	PORT=clientport;
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int k = 0;

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, neighbourno) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    //creating thread
    pthread_t tid;
    pthread_create(&tid, NULL, &receive_thread, &server_fd); //Creating thread to keep receiving message in real time
    chrono::milliseconds timespan(1); 
    this_thread::sleep_for(timespan);
    sending(clientid,neighbourport,neighbourno,clientno,clientport,files);
	
    while(1)
    {
    		if(neighbourno==-1){
    			break;
    		}
    		neighbourno--;
    		sleep(1);
    }

    //storing the received data in differnt array
    int n=received_data.size()/3;
    int received_data_number[n], received_data_id[n],received_data_port[n],neighbours_id[n];
	for (int i=0; i<n;i++)
    {
        received_data_number[i] = received_data[3*i];
        received_data_id[i] = received_data[3*i+1];
        neighbours_id[i] = received_data[3*i+1];    //for comparing the files in phase2
        received_data_port[i] = received_data[3*i+2];
    }
    
    //sorting the data with increasing order of neighbour number
    for(int i=0;i<n-1;i++)
    {
    	if(received_data_number[i] > received_data_number[i+1])
        {
            int temp1 = received_data_number[i];
            received_data_number[i] = received_data_number[i+1];
            received_data_number[i+1] = temp1;

            int temp2 = received_data_id[i];
            received_data_id[i] = received_data_id[i+1];
            received_data_id[i+1] = temp2;

            int temp3 = received_data_port[i];
            received_data_port[i] = received_data_port[i+1];
            received_data_port[i+1] = temp3;
            i=0;
        } 
    }
    
    for(int i=0;i<n;i++)
    {
        cout << "Connected to "<<received_data_number[i]<<" with unique-ID "<<received_data_id[i]<<" on port "<<received_data_port[i]<<endl;
    }
    //string received_files_array[received_files.size()];
    int files_number[n];int index=0;
    
    for(int i=0;i<sizeof(received_files);i++) {
        while(index<n) {
            files_number[index] = stringtoint(received_files[i]);
            i = i + 1 + files_number[index];
            index++;
        }
    }
    //finding the index of file number in files list as i had send files number corresponding to each number too
    int stamped_index[n];
    for(int i=0;i<n;i++) {
        stamped_index[i] = i;
        for(int j=0;j<i;j++) {
            stamped_index[i] = stamped_index[i]+files_number[j];
        }
        
    }

    //phase2 search file
    for (int i =0;i < filesearchno; i++) {
        vector<int> flag;
        for(int j=0;j<received_files.size();j++) {
            if(searchfile[i] == received_files[j]) {
                for(int k=0;k<n;k++) {
                    if(k!=n-1) {
                        if(j>stamped_index[k]&&j<stamped_index[k+1])
                            flag.push_back((neighbours_id[k]));
                    }
                    else
                        flag.push_back((neighbours_id[n-1]));
                }
            }
        }
        if(flag.size()==0)
            cout << "Found "<<searchfile[i]<<" at "<<"0"<<" with MD5 0 at depth "<<"0"<<endl;
        else if(flag.size()>0){
            int min = *min_element(flag.begin(), flag.end());
            cout << "Found "<<searchfile[i]<<" at "<<min<<" with MD5 0 at depth "<<"1"<<endl;
        }
    }
    
    
    //phase3
    
    close(server_fd);
    
    return 0;
}


//Sending messages to port
void sending(int clientid, int *neighbourport, int neighbourno,int clientno,int clientport,string files)
{

    char buffer[2000] = {0};
    int PORT_server;
    
    for (int i=0;i<neighbourno; i++)
    {
        PORT_server = neighbourport[i];

        int sock = 0, valread;
        struct sockaddr_in serv_addr;
        char hello[1024] = {0};
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("\n Socket creation error \n");
            return;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY; 
        serv_addr.sin_port = htons(PORT_server);

        while(1)
        {
        	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == 0)
        	{
            	break;
            }
            sleep(1);
        }
        const char *fileslist = files.c_str();
        
        sprintf(buffer, "%d %d %d %s", clientno, clientid, clientport, fileslist);
        send(sock, buffer, sizeof(buffer), 0);
        
        //this_thread::sleep_for(timespan);
        close(sock);
    }
}

void sending_back(int clientid,int clientno,int clientport,int neighbourport)
{

    char buffer[2000] = {0};
    int PORT_server;

    PORT_server = neighbourport;

    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char hello[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT_server);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        //printf("\nConnection Failed \n");
        return;
    }

    sprintf(buffer, "-1 %d %d %d", clientno, clientid, clientport);
    send(sock, buffer, sizeof(buffer), 0);
    close(sock);
}

//Calling receiving every 2 seconds
void *receive_thread(void *server_fd)
{
    int s_fd = *((int *)server_fd);
    while (1)
    {
        sleep(2);
        receiving(s_fd);
    }
}

//Receiving messages on our port
void receiving(int server_fd)
{
    struct sockaddr_in address;
    int valread;
    char buffer[2000] = {0};
    int addrlen = sizeof(address);
    fd_set current_sockets, ready_sockets;

    //Initialize my current set
    FD_ZERO(&current_sockets);
    FD_SET(server_fd, &current_sockets);
    int k = 0;
    while (1)
    {
        k++;
        ready_sockets = current_sockets;

        if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0)
        {
            perror("Error");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < FD_SETSIZE; i++)
        {
            if (FD_ISSET(i, &ready_sockets))
            {

                if (i == server_fd)
                {
                    int client_socket;

                    if ((client_socket = accept(server_fd, (struct sockaddr *)&address,
                                                (socklen_t *)&addrlen)) < 0)
                    {
                        perror("accept");
                        exit(EXIT_FAILURE);
                    }
                    FD_SET(client_socket, &current_sockets);
                }
                else
                {
                    valread = recv(i, buffer, sizeof(buffer), 0);
                    FD_CLR(i, &current_sockets);
                    string info(buffer);
                    string arr[200];

                    int g = 0;
                    stringstream ssin(info);
                    while (ssin.good() && g < 1)
                    {
                        ssin >> arr[g];
                        ++g;
                    }
                    
                    g = 0;
                    stringstream sssin(info);
                    while (sssin.good() && g < 200)
                    {
                        sssin >> arr[g];
                        ++g;
                    }
                    received_data.push_back(stoi(arr[0]));
                    received_data.push_back(stoi(arr[1]));
                    received_data.push_back(stoi(arr[2])); 
                    for (int j=3;j<200;j++){
                        if (arr[j]!=""&&arr[j]!=" ")
                            received_files.push_back(arr[j]);
                        else
                            break;
                    }      
                }
            }
        }

        if (k == (FD_SETSIZE * 2))
            break;
    }
}