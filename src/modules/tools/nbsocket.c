
    #ifndef nbsocket_h
    #define nbsocket_h

    #include <stdio.h>
    #include <unistd.h>

    typedef struct nbsocket_t nbsocket_t;
    struct nbsocket_t
    {
        int     socket;
        char    connected;
    };

    nbsocket_t* nbsocket_alloc(void);
    void        nbsocket_dealloc( void* pointer );
    int         nbsocket_connect( nbsocket_t* nbsocket , char* host , int port );
    void        nbsocket_close( nbsocket_t* nbsocket );
    int         nbsocket_timer( nbsocket_t* mqtt , char* readbuffer , ssize_t* readlength , char* sendbuffer , ssize_t* sendlength );

    #endif
    #if __INCLUDE_LEVEL__ == 0

	#include <fcntl.h>
	#include <errno.h>
	#include <netdb.h>
	#include <string.h>
	#include <assert.h>
    #include <sys/time.h>
    #include <sys/select.h>
	#include <sys/types.h>
	#include <sys/ioctl.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include "../core/mtmap.c"
	#include "../core/mtpipe.c"
	#include "../core/mtstr.c"
	#include "../core/mtcstr.c"


	/* alloc */

	nbsocket_t* nbsocket_alloc( )
	{
		nbsocket_t* result = mtmem_calloc( sizeof( nbsocket_t ) , nbsocket_dealloc );
		return result;
	}

	/* dealloc */

	void nbsocket_dealloc( void* pointer )
	{

	}

	/* gets family of remote address, creates sockaddr structure */

	struct sockaddr_in nbsocket_getsocketaddress( char* host , int port , int* success )
	{
		struct sockaddr_in client;
		struct hostent* hostentity = gethostbyname( host );

        memset( &client , 0 , sizeof( client ) );

		if ( hostentity != NULL )
		{
			*success = 1;
			memcpy( ( char* ) &client.sin_addr , hostentity->h_addr , hostentity->h_length );
			client.sin_family = hostentity->h_addrtype;
			client.sin_port = htons( port );
		}
		else *success = 0;
        
		return client;
	}
	
	/* disconnects socket from remote server */
		
	void nbsocket_close( nbsocket_t* nbsocket )
	{
		close( nbsocket->socket );
	}
	
	/* connects socket to remote server */

	int nbsocket_connect( nbsocket_t* nbsocket , char* host , int port )
	{
		int success = 0;
		
		nbsocket->socket = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP);
		
		if ( nbsocket->socket == -1 )
		{
			close(nbsocket->socket);
			printf( "Cannot create socket %s\n" , strerror(errno) );
			return -1;
		}
		
		success = fcntl( nbsocket->socket, F_SETFL, O_NONBLOCK );
		
		if ( success == -1 )
		{
			close(nbsocket->socket);
			printf( "Cannot set nonblocking mode for socket %s\n" , strerror(errno) );
			return -1;
		}

		struct sockaddr_in address = nbsocket_getsocketaddress( host , port , &success );
		
		if ( success == 0 )
		{
			close(nbsocket->socket);
			printf( "Cannot get address for hostname\n" );
			return -1;		
		}

		success = connect( nbsocket->socket , ( struct sockaddr* ) &address , sizeof( address ) );

		if ( errno == EINPROGRESS )
		{
			nbsocket->connected = 1;
		}
		else
		{
			printf( "Connection failed %s\n" , strerror(errno) );
			return -1;
		}
		
		return 0;
	}
	
	/* timer iteration, sends and receives available data */
	
	int nbsocket_timer( nbsocket_t* nbsocket , char* readbuffer , ssize_t* readlength , char* sendbuffer , ssize_t* sendlength )
	{
		if ( nbsocket->connected == 1 )
		{
			fd_set readset;
			fd_set sendset;
			
			FD_ZERO( &readset );
			FD_ZERO( &sendset );
			FD_SET( nbsocket->socket, &readset );
			FD_SET( nbsocket->socket, &sendset );
		  
			struct timeval tv;

			tv.tv_sec = 0;
			tv.tv_usec = 500000;

			int handles = select( 64, &readset, sendbuffer == NULL ? NULL : &sendset, NULL, &tv );

			if ( handles == -1 ) 
			{
				printf( "Select failed %s" , strerror(errno) );
				return -1;
			}
			
			/* recv stuff */

			if ( FD_ISSET( nbsocket->socket, &readset ) && readbuffer != NULL ) 
			{
				ssize_t count = recv( nbsocket->socket, readbuffer, *readlength, 0 );
				
				if ( count < 0 )
				{
					if ( errno == EWOULDBLOCK ) { *readlength = 0; }
					else if ( errno == EAGAIN ) { *readlength = 0; }
					else
					{
						printf("Recv error : %s" , strerror(errno) );
						return -1;
					}
				} 
				else *readlength = count; 
			}
			else *readlength = 0;

			/* send stuff */

			if ( FD_ISSET( nbsocket->socket, &sendset ) && sendbuffer != NULL ) 
			{
				ssize_t count = send( nbsocket->socket , sendbuffer , *sendlength , 0 );
				
				if ( count < 0 )
				{
					if ( errno == EWOULDBLOCK ) { *sendlength = 0; }
					else if ( errno == EAGAIN ) { *sendlength = 0; }
					else
					{
						printf("Send error : %s" , strerror(errno) );
						return -1;
					}			
				}
				else *sendlength = count;
			}
			else *sendlength = 0;
        }
		return 0;
	}

    #endif
