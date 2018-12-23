/*
 * This file is made available under the Creative Commons CC0 1.0 Universal Public Domain Dedication.
 * 
 * https://creativecommons.org/publicdomain/zero/1.0/
 * 
 */

/* 
 * File:   main.c
 * Author: ******
 *
 * Created on 22 декабря 2018 г., 0:10
 * 
 * 
 */

#define _GNU_SOURCE

#include <sched.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <netdb.h>
#include <pthread.h>

#include <getopt.h>


#define PORT "11000"
#define USEC 1000000
/*
 * 
 */

void sender(struct addrinfo *res_local, struct addrinfo *res_remote, unsigned int usecs);
void reciver(struct addrinfo *res_local, struct addrinfo *res_remote);

int main(int argc, char** argv) {
    /*
     * getifaddrs()
     * REUSEADDR
     * 
     * 
     * https://www.kernel.org/doc/Documentation/networking/timestamping.txt
     * http://man7.org/linux/man-pages/man3/cmsg.3.html
     * 
     * http://man7.org/linux/man-pages/man3/getaddrinfo.3.html
     * getaddrinfo(const char *node, const char *service,const struct addrinfo *hints,struct addrinfo **res);
     * AI_PASSIVE flag is specified in hints.ai_flags and node is NULL -  для ,bind на любой адрес (слушать)
     * типа взводим флаг hints_local.ai_flags=AI_PASSIVE, а вместо адреса интерфейса const char *node передаем NULL
     * и это должно годится для "local bind"
     * 
     * const char *service -  это кстати порт
     * 
     
     
     */

    struct addrinfo hints_local, hints_remote, *res_local, *res_remote;
    char *port = PORT;
    //char *source_ip = "0.0.0.0";
    char *source_ip = NULL;
    char *dst_ip = "0.0.0.0";
    unsigned int usecs = USEC;

    memset(&hints_local, 0, sizeof hints_local);
    memset(&hints_remote, 0, sizeof hints_remote);

    hints_local.ai_family = AF_UNSPEC;
    hints_remote.ai_family = AF_UNSPEC;
    hints_local.ai_socktype = SOCK_DGRAM;
    hints_remote.ai_socktype = SOCK_DGRAM;
    
    


    //SENDER
    if (argc > 1) {
        dst_ip=argv[1];
        
        int status;
        if ((status = getaddrinfo( source_ip, "0", &hints_local, &res_local)) != 0) {
            //if(errno!=0){printf("getaddrinfo local fails\n");perror("getaddrinfo local");
            fprintf(stderr, "getaddrinfo source_ip error: %s\n", gai_strerror(status));
            exit(EXIT_FAILURE);
        }


        if ((status = getaddrinfo(dst_ip, port, &hints_remote, &res_remote)) != 0) {
            fprintf(stderr, "getaddrinfo remote_ip error: %s\n", gai_strerror(status));
            exit(EXIT_FAILURE);
        }
        sender(res_local,res_remote,usecs);
        
    } else {


        //REPLYER
        int status;
        if ((status = getaddrinfo(source_ip, port, &hints_local, &res_local)) != 0) {
            fprintf(stderr, "getaddrinfo source_ip error: %s\n", gai_strerror(status));
            exit(EXIT_FAILURE);
        }
        reciver(res_local,res_remote);

    }

        return (EXIT_SUCCESS);
    }

    void sender(struct addrinfo *res_local, struct addrinfo *res_remote, unsigned int usecs) {

        int sock;
        unsigned char buf[50] = {0};

        sock = socket(res_remote->ai_family, res_remote->ai_socktype, res_remote->ai_protocol);
/*
        if (bind(sock, res_local->ai_addr, res_local->ai_addrlen) < 0) {
            perror("bind local addr");
            exit(EXIT_FAILURE);
        }
*/

        struct iovec iov[1];
        iov[0] .iov_base = (void*) buf;
        iov[0] .iov_len = sizeof (buf);


        struct msghdr msg;
        //res_remote->

        switch (res_remote->ai_family) {
            case AF_INET:
                msg.msg_name = (struct sockaddr_in *) res_remote->ai_addr;
                msg.msg_namelen = sizeof (struct sockaddr_in);
                break;
            case AF_INET6:
                msg.msg_name = (struct sockaddr_in6 *) res_remote->ai_addr;
                msg.msg_namelen = sizeof (struct sockaddr_in6);

                break;
        }


        msg.msg_iov = iov;
        msg.msg_iovlen = 1;

/*
        struct cmsghdr cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SO_TIMESTAMPING;
    cmsg->cmsg_len = CMSG_LEN(sizeof (__u32));
    *((__u32 *) CMSG_DATA(cmsg)) = SOF_TIMESTAMPING_TX_SCHED |
            SOF_TIMESTAMPING_TX_SOFTWARE |
            SOF_TIMESTAMPING_TX_ACK;
  */                
    while (1) {
        errno = 0;
        int k = sendmsg(sock, &msg, 0);
        if (k < 1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                perror("sendmsg");
            } else {
                perror("sendmsg");
                return;
            }
        }
        fprintf(stderr,".");
        usleep(usecs);
    }


    }

    void reciver(struct addrinfo *res_local, struct addrinfo * res_remote) {
        int sock;
        unsigned char buf[50] = {0};

        sock = socket(res_local->ai_family, res_local->ai_socktype, res_local->ai_protocol);

        if (bind(sock, res_local->ai_addr, res_local->ai_addrlen) < 0) {
            perror("bind local addr");
            exit(EXIT_FAILURE);
        }

        struct msghdr msg;
        struct iovec iov[1];
        switch (res_local->ai_family) {
            case AF_INET:
                msg.msg_name = (struct sockaddr_in *) res_local->ai_addr;
                msg.msg_namelen = sizeof (struct sockaddr_in);
                break;
            case AF_INET6:
                msg.msg_name = (struct sockaddr_in6 *) res_local->ai_addr;
                msg.msg_namelen = sizeof (struct sockaddr_in6);

                break;
        }
        iov[0] .iov_base = (void*) buf;
        iov[0] .iov_len = sizeof (buf);

        msg.msg_iov = iov;
        msg.msg_iovlen = 1;

        while (1) {
            errno = 0;
            int r = recvmsg(sock, &msg, 0);

            if (r <= 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                    continue;
                }
                perror("recvmsg()");
                continue;
            }


            {

                char *s = inet_ntoa(((struct sockaddr_in *) msg.msg_name)->sin_addr);
                printf("RCV FROM IP address: %s\n", s);
                
                
                char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
                if (getnameinfo(((struct sockaddr *) msg.msg_name), msg.msg_namelen, hbuf, sizeof(hbuf), sbuf,
                       sizeof(sbuf), 
                        //NI_NUMERICHOST | NI_NUMERICSERV
                        0
                        ) == 0)
               printf("host=%s, serv=%s\n", hbuf, sbuf);
                // */
                
            }

        }
    }

