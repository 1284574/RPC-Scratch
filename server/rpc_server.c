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

#include "is_prime.h"

#define SERVERPORT "8001" // port the server will be listening on

// Gets the IPv4 or IPv6 sockaddr

void *get_in_addr(struct sockaddr *sa)
{
    if(sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in*) sa) -> sin_addr);
    }
    else
    {
        return &(((struct sockaddr_in6*) sa)-> sin6_addr);
    }
}
// unpacks an int, need to convert it from network order to our host order (endianess)
int unpack(int packed)
{
    return ntohs(packed);
}

// gets a socket to listen with
int get_and_bind_socket()
{
    int sockfd;
    struct addrinfo hints, *server_info, *p;
    int number_of_bytes;

    // Given node and service, which identify an Internet host and a
    //    service, getaddrinfo() returns one or more addrinfo structures,
    //    each of which contains an Internet address that can be specified
    //    in a call to bind(2) or connect(2).  The getaddrinfo() function
    //    combines the functionality provided by the gethostbyname(3) and
    //    getservbyname(3) functions into a single interface, but unlike the
    //    latter functions, getaddrinfo() is reentrant and allows programs
    //    to eliminate IPv4-versus-IPv6 dependencies.
    // The hints argument points to an addrinfo structure that specifies
    //    criteria for selecting the socket address structures returned in
    //    the list pointed to by res.  If hints is not NULL it points to an
    //    addrinfo structure whose ai_family, ai_socktype, and ai_protocol
    //    specify criteria that limit the set of socket addresses returned
    //    by getaddrinfo(), as follows:

    // ai_family: This field specifies the desired address family for the returned addresses.
    // ai_socktype: This field specifies the preferred socket type
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM; // use TCP to ensure reliability
    hints.ai_flags = AI_PASSIVE; // use server's ip address
    int rv = getaddrinfo(NULL, SERVERPORT, &hints, &server_info);
    if(rv != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    // have a linked list of addresses and we want to connect to first one we can
    for(p = server_info; p != NULL; p = p ->ai_next)
    {
        // try to make a socket
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            // something went wrong with socket, try next one
            perror("server: socket");
            continue;
        }
        int reuse = 1; // want to be able to reuse, so we can set the socket option
        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1)
        {
           perror("setsockopt");
           exit(1);
        }
        // bind to socket
        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            // if something went wrong binding to socket, we can close it and move on to next one
            close(sockfd);
            perror("server: bind");
            continue;
        }
        // If we made it this far, we have a valid socket and can stop iterating through
        break; 
    }

    // If we do not have a valid sockaddr here, that means we can't connect.
    if(p == NULL)
    {
        fprintf(stderr, "server: failed to bind\n");
        exit(2);
    }
    // otherwise we're good
    return sockfd;
}

int main(void)
{
    int sockfd = get_and_bind_socket();
    // we want to listen forever on this socket
    if(listen(sockfd, /*backlog*/1) == -1)
    {
        perror("listen");
        exit(1);
    }
    printf("Server waiting for connections.\n");

    struct sockaddr client_addr; // address info of client
    socklen_t sin_size;
    int new_fd;
    while(1)
    {
        sin_size = sizeof(client_addr);
        new_fd = accept(sockfd, (struct sockaddr*)&client_addr, &sin_size);
        if(new_fd == -1)
        {
            perror("accept");
            continue;
        }

        // once we have accepted an incoming request, we can read from it into a buffer
        int buffer;
        int bytes_received = recv(new_fd, &buffer, sizeof(buffer), 0);
        if(bytes_received == -1)
        {
            perror("recv");
            continue;
        }
        // need to unpack the received data
        int number = unpack(buffer);
        printf("Received a request: is %d prime?\n", number);

        // call is_prime library
        bool prime_number = is_prime(number);
        printf("Sending response: %s\n", prime_number ? "true" : "false");
        // don't have to pack a single byte

        // send it back
        if(send(new_fd, &prime_number, sizeof(prime_number), 0) == -1)
        {
            perror("send");
        }
        close(new_fd);
    }
}