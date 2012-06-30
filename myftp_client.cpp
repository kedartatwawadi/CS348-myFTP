/**********************************************************************************************
 Assignment2 - Computer_networks
 Lab Group:
 
 1) Ayesha Mudassir [09007014]
 2) Kedar Tatwawadi [09D07022]
 
 
 myftp_client.cpp -- a minimalistic ftp client
  the code snippets were adapted from various sources:
 1) http://cboard.cprogramming.com/c-programming/102068-copy-read-write-binary-file-mp3.html
   (cprogramming.com)
 2) http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
   (beej programming tutorial)
************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fstream>
#include <iostream>
#include <string>

using namespace std; 

#define MAXDATASIZE 1000000 // max number of bytes we can get at once 


// get sockaddr, IPv4 or IPv6:
/***********************************************************************************************************************
struct sockaddr 
{
    unsigned short    sa_family;    // address family, AF_xxx
    char              sa_data[14];  // 14 bytes of protocol address
}; 
***********************************************************************************************************************/
/***************************************************************************************************************
		struct sockaddr_in 
		{
	    short int          sin_family;  // Address family, AF_INET
    	unsigned short int sin_port;    // Port number
    	struct in_addr     sin_addr;    // Internet address
    	unsigned char      sin_zero[8]; // Same size as struct sockaddr
		};
		
		struct in_addr 
		{
	    uint32_t s_addr; // that's a 32-bit int (4 bytes)
		};
		***************************************************************************************************************/
		/******************************************************************************************************************
	struct sockaddr_in6 
	{
    u_int16_t       sin6_family;   // address family, AF_INET6
    u_int16_t       sin6_port;     // port number, Network Byte Order
    u_int32_t       sin6_flowinfo; // IPv6 flow information
    struct in6_addr sin6_addr;     // IPv6 address
    u_int32_t       sin6_scope_id; // Scope ID
	};

	struct in6_addr 
	{
    unsigned char   s6_addr[16];   // IPv6 address
	};
	******************************************************************************************************************/
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) 
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
		
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
	
}




int main(int argc, char* argv[])
{
	ofstream desstr;
	char * buffer;
	long size;
	int sockfd, numbytes;  
	char flag;
	
	struct addrinfo hints, *servinfo, *p;	//servinfo - points to the result
	/*********************************************************************************************************************
	struct addrinfo 
	{
    int              ai_flags;     // AI_PASSIVE, AI_CANONNAME, etc.
    int              ai_family;    // AF_INET, AF_INET6, AF_UNSPEC
    int              ai_socktype;  // SOCK_STREAM, SOCK_DGRAM
    int              ai_protocol;  // use 0 for "any"
    size_t           ai_addrlen;   // size of ai_addr in bytes
    struct sockaddr *ai_addr;      // struct sockaddr_in or _in6
    char            *ai_canonname; // full canonical hostname

    struct addrinfo *ai_next;      // linked list, next node
	};
	**********************************************************************************************************************/
	int rv; //return value
	char s[INET6_ADDRSTRLEN];

	if (argc != 4) 
	{
	    fprintf(stderr,"usage: myftp_client <server_ip> <server_port> <file_name>\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints); //make sure the struct is empty
	hints.ai_family = AF_UNSPEC; //Don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; //TCP stream sockets

	if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) 
	/**********************************************************************************************************************
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netdb.h>

	int getaddrinfo(const char *node,     			// host name to connect to or IP
    	            const char *service,  			// e.g. "http" or port number
    	            const struct addrinfo *hints,	//hints parameter points to a struct addrinfo that u've already filled out with relevant info
    	            struct addrinfo **res);
   	**********************************************************************************************************************/
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
		/******************************************************************************************************************
		#include <sys/types.h>
		#include <sys/socket.h>

		int socket(int domain, int type, int protocol); 
		socket() simply returns to you a socket descriptor that you can use in later system calls, or -1 on error. 
		******************************************************************************************************************/
		{
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) //sockfd - socket descriptor returned by socket()
		{	
			close(sockfd);
			perror("client: connect");
			continue;
		}
		
		/******************************************************************************************************************
		#include <sys/types.h>
		#include <sys/socket.h>

		int connect(int sockfd, struct sockaddr *serv_addr, int addrlen); 
		******************************************************************************************************************/

		break;
	}

	if (p == NULL) 
	{
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),	s, sizeof s);
	/******************************************************************************************************************
	ntop = network to presentation
		
	// IPv4:
	char ip4[INET_ADDRSTRLEN];  // space to hold the IPv4 string
	struct sockaddr_in sa;      // pretend this is loaded with something
	inet_ntop(AF_INET, &(sa.sin_addr), ip4, INET_ADDRSTRLEN);
	printf("The IPv4 address is: %s\n", ip4);

	// IPv6:
	char ip6[INET6_ADDRSTRLEN]; // space to hold the IPv6 string
	struct sockaddr_in6 sa6;    // pretend this is loaded with something
	inet_ntop(AF_INET6, &(sa6.sin6_addr), ip6, INET6_ADDRSTRLEN);
	printf("The address is: %s\n", ip6);
	******************************************************************************************************************/
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure
	
  //Send the file name	
	send(sockfd, argv[3] ,strlen(argv[3]), 0);
	/******************************************************************************************************************
	int send(int sockfd, const void *msg, int len, int flags); 
	returns the number of bytes actually sent out, -1 on error
	******************************************************************************************************************/
	
	//Get info whether the file exists or not
	recv(sockfd,&flag,1, 0);
	
	if (flag == '1')// file exists!
	{
	
		//Open the file to write
		desstr.open(argv[3], ios::out | ios::binary | ios::app);
		
		
		//Get the size of file
		int l = recv(sockfd,&size,sizeof(long), 0);
		/**********************************************************************************************************************
		int recv(int sockfd, void *buf, int len, int flags);
		returns the number of bytes actually read into the buffer, -1 on error
		**********************************************************************************************************************/
		
		buffer=new char[5000];
		
		long N = size/5000;
		
		//write to file
		for (long i = 0; i<N; i++)
		{
			l=recv(sockfd, buffer, 5000, 0);
			desstr.write(buffer,5000);
		}
		recv(sockfd, buffer, size-N*5000, 0);
		desstr.write(buffer,size-N*5000);
		
		printf("File succesfully received\n");
		
		//close the file
		desstr.close();
	}
		
	else // file doesnt exist on the server
	{
		printf("File '%s' doesn't exist\n",argv[3]);
	}
	
	//close socket
	close(sockfd);

	return 0;
}

