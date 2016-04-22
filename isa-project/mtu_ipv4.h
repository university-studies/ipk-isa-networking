/**
 * File: mtu_ipv4.h
 * Author: Pavol Loffay, xloffa00@stud.fit.vutbr.cz
 * Date: 25.9.2012
 * Description: modul spracujuci path MTU discovery pre IPv4
 */

#ifndef MTU_IPV4_H
#define MTU_IPV4_H

#include <string>

#include <netdb.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>

class mtu_ipv4 
{
    public:
        mtu_ipv4(struct sockaddr_in h_sockaddr, int max_mtu) throw (const char *);
        ~mtu_ipv4();
        int calculate(void) throw( const char *);

    private:
        int max_mtu;
        int min_mtu;
        protoent * protocol;
        int id;
        int sequence;
        int packet_len;
        int socket_id;
        struct iphdr * ip_send;
        struct iphdr * ip_recv;
        struct icmphdr * icmp_send;
        struct icmphdr * icmp_recv; 
        char * recv_packet;
        char * send_packet;
        struct sockaddr_in send_sockaddr;
        struct sockaddr_in recv_sockaddr;
        //struct hostent * host;
        timeval time;

        void make_icmp(void);
        void make_ip(void);
        bool fragment_need(unsigned int len);
    enum
    {
        MIN_MTU = 68, //68 ,, hostovia mozu prijat 576bytes datagrami rfc791
        MAX_MTU = 1500 + 1, //1500
        MAX_TTL = 30,
        TIMEOUT_S = 3,
        DF = 16384
    };
};

#endif // MTU_IPV4_H
