/**********************************************************************************************
 Assignment2 - Computer_networks
 Lab Group:
 
 1) Ayesha Mudassir [09007014]
 2) Kedar Tatwawadi [09D07022]
 
 
 myftp_server.cpp -- a minimalistic ftp server
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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <fstream>
#include <iostream>

using namespace std;


											// The Port number to be given as an argument.
#define BACKLOG 10   // how many pending connections queue will hold

//the handler called on SIGCHILD
void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
/**************************************************************************************************************************
struct sockaddr 
{
    unsigned short    sa_family;    // address family, AF_xxx
    char              sa_data[14];  // 14 bytes of protocol address
}; 
**************************************************************************************************************************/
/******************************************************************************************************************
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
		******************************************************************************************************************/
		/**********************************************************************************************************************
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
	**********************************************************************************************************************/

void *get_in_addr(struct sockaddr *sa)

{
	if (sa->sa_family == AF_INET) 
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
		
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
	
}





int main(int argc, char *argv[])
{
	filebuf *pbuf;
	ifstream sourcestr;
	long size, N;
  char * buffer;
    
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;	//servinfo - points to the result
	/**********************************************************************************************************************
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
	struct sockaddr_storage their_addr; // connector's address information
	/**********************************************************************************************************************
	struct sockaddr_storage 		//desined to hold both IPv4 and IPv6 structures
	{
    sa_family_t  ss_family;     	// address family	
    }
    **********************************************************************************************************************/
    
	
	
	socklen_t sin_size;
	struct sigaction sa;//for reaping zombie processes that appear as the fork()ed child processes exit
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv; //return value
	
	if (argc != 2) 
	{
	    fprintf(stderr,"usage: myftp_server <port_no>\n");
	    exit(1);
	}
	
	memset(&hints, 0, sizeof hints); //make sure the struct is empty
	hints.ai_family = AF_UNSPEC; //don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; //TCP Stream Sockets
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) //0 = success, !0 = error
	/**********************************************************************************************************************
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netdb.h>

	int getaddrinfo(const char *node,     			// host name to connect to or IP
    	            const char *service,  			// e.g. "http" or port number
    	            const struct addrinfo *hints,	//hints parameter points to a struct addrinfo that u've already filled out with relevant info
    	            struct addrinfo **res);
    	            
    	            
 	Get information about a host name and/or service and load up a struct sockaddr with the result.
 	Can pass NULL as bind() will fill in for us later
 	The res will now point to a linked list of struct addrinfos,can go through this list to get all the addresses that match what you passed in with the hints.
   	**********************************************************************************************************************/
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
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
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) //allows reusing the port
		{
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) //sockfd - socket fiel descriptor returned by socket()
		/******************************************************************************************************************
		#include <sys/types.h>
		#include <sys/socket.h>

		int bind(int sockfd, struct sockaddr *my_addr, int addrlen);
		******************************************************************************************************************/
		{
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL)  
	{
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (listen(sockfd, BACKLOG) == -1) 
	/**********************************************************************************************************************
	int listen(int sockfd, int backlog); 
	Tell a socket to listen for incoming connections	
	**********************************************************************************************************************/
	{
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) 
	{
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while(1) 
	{  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size); //sort of fork
		/******************************************************************************************************************
		#include <sys/types.h>
		#include <sys/socket.h>

		int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen); 
		Accept an incoming connection on a listening socket
		******************************************************************************************************************/
		if (new_fd == -1) 
		{
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
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
		printf("server: got connection from %s\n", s);

		
		if (!fork()) 
		{ // this is the child process (fork() returns 0 for child process	
			//Get the file name
			char str[100];
			int num = recv(new_fd, str, 255, 0);
			/**********************************************************************************************************************
			int recv(int sockfd, void *buf, int len, int flags);
			returns the number of bytes actually read into the buffer, -1 on error
			**********************************************************************************************************************/
			str[num] = '\0';
			
			sourcestr.open(str,ios::in | ios::binary);
			
			if (sourcestr.is_open())
			{
				send(new_fd, "1", 1, 0);
				// the flag '1' is sent if the file exists.. else.. '0' is sent
				// get pointer to associated buffer object
 				pbuf=sourcestr.rdbuf();
 	
				// get file size using buffer's members
  				size=pbuf->pubseekoff (0,ios::end,ios::in);
				pbuf->pubseekpos (0,ios::in);
				N = size/5000;
 
				// allocate memory
				// the buffer size has been set to 5000.. can be changed.
				buffer=new char[5000];
				
		
				//send the file size
				send(new_fd, &size, sizeof(long), 0);
				/******************************************************************************************************************
				int send(int sockfd, const void *msg, int len, int flags); 
				returns the number of bytes actually sent out, -1 on error
				******************************************************************************************************************/
 				
 				//send the file
 				for (long i=0; i<N; i++)
 				{
 					pbuf->sgetn (buffer,5000);
					send(new_fd, buffer,5000, 0);
				}
				pbuf->sgetn (buffer,size-N*5000);
				send(new_fd, &buffer[N*5000],size-N*5000, 0);
				
				sourcestr.close();
			}
			
			else 
			{
				send(new_fd, "0", 1, 0);// file doesnt exist
			}
		}
		close(new_fd);  // parent doesn't need this
	}

	return 0;
}


/*--------------------------------------------------------END-----------------------------------------------------------------*/
