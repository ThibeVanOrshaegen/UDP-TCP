//libraries for windows
#ifdef _WIN32
#define _WIN32_WINNT _WIN32_WINNT_WIN7
#include <winsock2.h> //for all socket programming
#include <ws2tcpip.h> //for getaddrinfo, inet_pton, inet_ntop
#include <stdio.h> //for fprintf, perror
#include <unistd.h> //for close
#include <stdlib.h> //for exit
#include <string.h> //for memset
void OSInit( void ) //initializes the Windows Sockets API
{
    WSADATA wsaData;
    int WSAError = WSAStartup( MAKEWORD( 2, 0 ), &wsaData );
    if( WSAError != 0 )
    {
        fprintf( stderr, "WSAStartup errno = %d\n", WSAError );
        exit( -1 );
    }
}
void OSCleanup( void )
{
    WSACleanup();
}
#define perror(string) fprintf( stderr, string ": WSA errno = %d\n", WSAGetLastError() )

//library's for linux
#else
#include <sys/socket.h> //for sockaddr, socket, socket
#include <sys/types.h> //for size_t
#include <netdb.h> //for getaddrinfo
#include <netinet/in.h> //for sockaddr_in
#include <arpa/inet.h> //for htons, htonl, inet_pton, inet_ntop
#include <errno.h> //for errno
#include <stdio.h> //for fprintf, perror
#include <unistd.h> //for close
#include <stdlib.h> //for exit
#include <string.h> //for memset
int OSInit( void ) {}
int OSCleanup( void ) {}
#endif

#include <time.h>
#include <stdint.h>

int initializationUDP();
void executionUDP( int internet_socket );
void cleanupUDP( int internet_socket );
//int randomNumberUDP();
//void sendNumberUDP(int internet_socket,struct sockaddr_storage client_internet_address,socklen_t client_internet_address_length,int number_of_bytes_send);
//void receiveNumberUDP(char buffer[1000],int internet_socket,struct sockaddr_storage client_internet_address,socklen_t client_internet_address_length,int number_of_bytes_received);
void Randomnummer(int internet_socket,struct sockaddr_storage client_internet_address,socklen_t client_internet_address_length,int number_of_bytes_send );



int main( int argc) {
    //UDP

    srand(time(NULL));

    //initialize the socket and the OS
    OSInit();

    int internet_socketUDP = initializationUDP();

   struct timeval timeout;
     timeout.tv_sec = 3;
     timeout.tv_usec = 0;
     if (setsockopt(internet_socketUDP, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout,sizeof timeout) < 0){
         perror("setsockopt failed\n");
     }

    //execute the code for the server

    executionUDP(internet_socketUDP);

    //clean everything

    cleanupUDP(internet_socketUDP);

    OSCleanup();

    //END UDP

    return 0;
}

int initializationUDP()
{
    //set up the internet address
    struct addrinfo internet_address_setup;
    struct addrinfo * internet_address_result;
    memset( &internet_address_setup, 0, sizeof internet_address_setup );
    internet_address_setup.ai_family = AF_UNSPEC;
    internet_address_setup.ai_socktype = SOCK_DGRAM;
    internet_address_setup.ai_flags = AI_PASSIVE;
    int getaddrinfo_return = getaddrinfo( NULL, "24060", &internet_address_setup, &internet_address_result );
    if( getaddrinfo_return != 0 )
    {
        fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( getaddrinfo_return ) );
        exit( 1 );
    }

    int internet_socket = -1;
    struct addrinfo * internet_address_result_iterator = internet_address_result;
    while( internet_address_result_iterator != NULL )
    {
        //open the socket
        internet_socket = socket( internet_address_result_iterator->ai_family, internet_address_result_iterator->ai_socktype, internet_address_result_iterator->ai_protocol );
        if( internet_socket == -1 )
        {
            perror( "socket" );
        }
        else
        {
            //bind
            int bind_return = bind( internet_socket, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen );
            if( bind_return == -1 )
            {
                close( internet_socket );
                perror( "bind" );
            }
            else
            {
                break;
            }
        }
        internet_address_result_iterator = internet_address_result_iterator->ai_next;
    }

    freeaddrinfo( internet_address_result );

    if( internet_socket == -1 )
    {
        fprintf( stderr, "socket: no valid socket address found\n" );
        exit( 2 );
    }

    return internet_socket;
}

void executionUDP( int internet_socket )
{
    //declare parameters for the execution
    int number_of_bytes_send = 0;
    int number_of_bytes_received = 0;
    char buffer[1000];
    struct sockaddr_storage client_internet_address;
    socklen_t client_internet_address_length = sizeof client_internet_address;

    //need to receive GO before sending the numbers
    while (strcmp(buffer,"GO")!=0){
        number_of_bytes_received = recvfrom( internet_socket, buffer, ( sizeof buffer ) - 1, 0, (struct sockaddr *) &client_internet_address, &client_internet_address_length );
        if( number_of_bytes_received == -1 )
        {
            perror( "recvfrom" );
        }
        else
        {
            buffer[number_of_bytes_received] = '\0';
            printf( "Received : %s\n", buffer );
        }
    }

    //send 42 random numbers
    Randomnummer(internet_socket,client_internet_address,client_internet_address_length,number_of_bytes_send);

    //receive the highest number from the client and check if it was the highest number the server send
    number_of_bytes_received = recvfrom( internet_socket, buffer, (sizeof buffer) -1, 0, (struct sockaddr *) &client_internet_address, &client_internet_address_length );
    if( number_of_bytes_received == -1 )
    {
        perror( "recvfrom" );
    }
    else
    {
        buffer[number_of_bytes_received] = '\0';
        //printf( "Received Heximaal : %hd\n", ntohs(*((short*)buffer)) );
	  //printf("Received Decimaal: %d\n",buffer);
	  printf("Received String: %s\n", buffer);
	  //printf("Received Omgkeerd: %d\n",ntohs(*(buffer)));
    }

    //send 42 random numbers
    Randomnummer(internet_socket,client_internet_address,client_internet_address_length,number_of_bytes_send);

    //receive the highest number from the client and check if it was the highest number the server send
    number_of_bytes_received = recvfrom( internet_socket, buffer, (sizeof buffer) -1, 0, (struct sockaddr *) &client_internet_address, &client_internet_address_length );
    if( number_of_bytes_received == -1 )
    {
        perror( "recvfrom" );
    }
    else
    {
        buffer[number_of_bytes_received] = '\0';
        //printf( "Received Heximaal : %hd\n", ntohs(*((short*)buffer)) );
	  //printf("Received Decimaal: %d\n",buffer);
	  printf("Received String: %s\n", buffer);
	  //printf("Received Omgkeerd: %d\n",ntohs(*(buffer)));
	
    }

    //if the client send the highest number 2 times correctly the server sends OK back
    number_of_bytes_send = sendto( internet_socket,"OK",2, 0, (struct sockaddr *) &client_internet_address, client_internet_address_length );
    if( number_of_bytes_send == -1 )
    {
        perror( "sendto" );
    }
}

void cleanupUDP( int internet_socket )
{
    //close the socket
    close( internet_socket );
}

void Randomnummer(int internet_socket,struct sockaddr_storage client_internet_address,socklen_t client_internet_address_length,int number_of_bytes_send )
{
  uint16_t getal = 0;
  uint16_t getalnet = htons(getal);
  uint16_t max_getal = 0; // houd het maximale getal bij
  int i;
  srand(time(NULL));
  for(i = 0; i < 42; i++)
    {
      getal = rand() % 100;
    
      printf("Random getal is %d\n",getal);
if (getal > max_getal) {
max_getal = getal; // update het maximale getal
}
      getalnet = htons(getal);
      number_of_bytes_send = sendto( internet_socket, (const char*)&getalnet, sizeof(getalnet), 0, (struct sockaddr *) &client_internet_address, client_internet_address_length );
   if( number_of_bytes_send == -1 )
        {
            perror( "sendto" );
        }
}
printf("Maximale getal is %d\n", max_getal); // print het maximale getal
}