#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVERPORT "8001" // port server will listen on
#define SERVER "localhost" // localhost (loopback IP address)

#include "rpc_client.h"

// packs an int, we need to convert it from host order to network order
// byte ordering (endianess) matters when transferring across network
int pack(int input)
{
    return htons(input); // converts unsigned int from host byte order to network byte order
}

// get ipv4 or ipv6 sockaddr
void *get_in_addr(struct sockaddr *sa)
{
    if(sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in*) sa) -> sin_addr);
    }
    else
    {
        return &(((struct sockaddr_in6*) sa) -> sin6_addr);
    }
}

// get a socket to connect with
int get_socket()
{
    int sockfd;
    struct addrinfo hints, *server_info, *p;
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
    int number_of_bytes;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM; // want to use TCP to ensure it gets there (reliability)
    int return_value = getaddrinfo(SERVER, SERVERPORT, &hints, &server_info);
    if(return_value != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(return_value));
        exit(1);
    }

    // we have a linked list of addresses, and we want to connect to the first one we can
    for(p = server_info; p != NULL; p->ai_next)
    {
        // try to make a socket with the address
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            // something went wrong getting the socket
            perror("client: socket");
            continue;
        }
        // try to connect to that socket
        if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            // if something went wrong, close socket, we can close it and move on to next one
            close(sockfd);
            perror("client: connect");
            continue;
        }
        // we now have a valid socket and can stop iteration
        break;
    }

    // if we do not have a valid sockadddr here, means we cannot connect
    if(p == NULL)
    {
        fprintf(stderr, "client: failed to connect\n");
        exit(2);
    }

    return sockfd; // return file descriptor for socket
}

// client-side library for RPC function
bool is_prime_rpc(int num)
{
    // first need to pack data, ensuring that it's sent across the network in the right format
    int packed = pack(num);
    // grab a socket we can use to connect, see if we can connect
    int sockfd = get_socket();
    // send just packed number
    if(send(sockfd, &packed, sizeof(packed), 0) == -1)
    {
        perror("send");
        close(sockfd);
        exit(0);
    }

    // wait to receive anwser
    int buf[1]; // receive a single byte back which represents a boolean
    int bytes_received = recv(sockfd, &buf, 1, 0);
    if(bytes_received == -1)
    {
        perror("recv");
        exit(1);
    }

    bool result = buf[0]; // store byte anwser as our result
    // done, close socket and return result
    close(sockfd);
    return result;

}
