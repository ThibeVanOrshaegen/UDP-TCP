#ifdef _WIN32
#define _WIN32_WINNT _WIN32_WINNT_WIN7
#include <winsock2.h> //for all socket programming
#include <ws2tcpip.h> //for getaddrinfo, inet_pton, inet_ntop
#include <stdio.h> //for fprintf, perror
#include <unistd.h> //for close
#include <stdlib.h> //for exit
#include <string.h> //for memset
void OSInit( void )
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
	void OSInit( void ) {}
	void OSCleanup( void ) {}
#endif

#include <stdint.h>
#include <time.h>
#include <math.h>

int initializationUDP( struct sockaddr ** internet_address, socklen_t * internet_address_length );
void executionUDP( int internet_socket, struct sockaddr * internet_address, socklen_t internet_address_length );
void cleanupUDP( int internet_socket, struct sockaddr * internet_address );

int initializationTCP();
void executionTCP( int internet_socket );
void cleanupTCP( int internet_socket );
uint16_t randomNumberTCP();
char randomOperationTCP();
void checkAnswerTCP(char buffer[100],float number1,float number2,char operation);

int main(int argc, char *argv[]) {

    srand(time(NULL));

    //START UDP
    printf("Start UDP client\n");
    printf("\n");

    OSInit();


    struct sockaddr *internet_address = NULL;
    socklen_t internet_address_length = 0;
    int internet_socketUDP = initializationUDP(&internet_address, &internet_address_length);

    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    if (setsockopt(internet_socketUDP, SOL_SOCKET, SO_RCVTIMEO, &timeout,sizeof timeout) < 0){
        perror("setsockopt failed\n");
    }



    executionUDP(internet_socketUDP, internet_address, internet_address_length);

    cleanupUDP(internet_socketUDP, internet_address);

    OSCleanup();
    //END UDP
    printf("\n");
    printf("all ended well\n");

    return 0;
}

int initializationUDP( struct sockaddr ** internet_address, socklen_t * internet_address_length )
{
    //Step 1.1
    struct addrinfo internet_address_setup;
    struct addrinfo * internet_address_result;
    memset( &internet_address_setup, 0, sizeof internet_address_setup );
    internet_address_setup.ai_family = AF_UNSPEC;
    internet_address_setup.ai_socktype = SOCK_DGRAM;
    int getaddrinfo_return = getaddrinfo( "::1", "24060", &internet_address_setup, &internet_address_result );
    if( getaddrinfo_return != 0 )
    {
        fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( getaddrinfo_return ) );
        exit( 1 );
    }

    int internet_socket = -1;
    struct addrinfo * internet_address_result_iterator = internet_address_result;
    while( internet_address_result_iterator != NULL )
    {
        //Step 1.2
        internet_socket = socket( internet_address_result_iterator->ai_family, internet_address_result_iterator->ai_socktype, internet_address_result_iterator->ai_protocol );
        if( internet_socket == -1 )
        {
            perror( "socket" );
        }
        else
        {
            //Step 1.3
            *internet_address_length = internet_address_result_iterator->ai_addrlen;
            *internet_address = (struct sockaddr *) malloc( internet_address_result_iterator->ai_addrlen );
            memcpy( *internet_address, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen );
            break;
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

void executionUDP(int internet_socket, struct sockaddr *internet_address, socklen_t internet_address_length) {
    int num_loops = 2;
    int num_responses = 0;
    uint16_t buffer[42];
    int number_of_bytes_received = 0;
    uint16_t highest_number = 0;
    uint16_t current_number = 0;
    uint16_t previous_number = 0;
    uint16_t highest_net_number = 0;
    int i = 0;


    int number_of_bytes_send = sendto(internet_socket, "GO", 2, 0, internet_address, internet_address_length);
    if (number_of_bytes_send == -1) {
        perror("sendto");
    }

    while (num_responses < num_loops) {

        for ( i = 0; i < 42; i++) {
            number_of_bytes_received = recvfrom(internet_socket, (char *)&(buffer[i]), sizeof(buffer[i]), 0, NULL, NULL);
            if (number_of_bytes_received == -1) {
                perror("recvfrom");
                break;
            } else
            {
                //buffer[number_of_bytes_received] = '\0';
                printf( "Received : %d\n", ntohs(buffer[i]) );
            }
        }
        if(i != 0)
        {
            num_responses++;
            highest_number = 0;


            for ( i = 0; i < 42; i++) {
                current_number = ntohs(buffer[i]);
                //printf("number = %d\n",current_number);
                if (current_number > highest_number) {
                    highest_number = current_number;
                }
            }

        }
        highest_net_number = htons(highest_number);
        while (1) {
        number_of_bytes_send = sendto(internet_socket, (const char*)&highest_net_number, sizeof(highest_net_number), 0, internet_address, internet_address_length);
        if (number_of_bytes_send == -1) {
            perror("sendto");
            break;
        }
    }

    number_of_bytes_received = recvfrom(internet_socket, (char *)&buffer, sizeof(buffer), 0, NULL, NULL);
    if (number_of_bytes_received == -1) {
        perror("recvfrom");
    } else
    {
        buffer[number_of_bytes_received] = '\0';
        printf( "Received : %s\n", buffer );
    }
}


void cleanupUDP( int internet_socket, struct sockaddr * internet_address )
{
    //Step 3.2
    free( internet_address );

    //Step 3.1
    close( internet_socket );
} 