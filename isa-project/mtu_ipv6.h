/**
 * File: mtu_ipv6.h
 * Author: Pavol Loffay, xloffa00@stud.fit.vutbr.cz
 * Date: 25.9.2012
 * Description: modul spracujuci checksum protolu TCP, UDP
 */

#ifndef MTU_IPV6_H
#define MTU_IPV6_H

#include <string>

#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <netdb.h>

/*
 * @brief Trieda spracujuca MTU path discovery pre IPv6
 */
class mtu_ipv6
{
    public:
        mtu_ipv6(struct sockaddr_in6 h_sockaddr, int max_mtu) throw (const char *);
        ~mtu_ipv6();
        int calculate() throw (const char *);

    private:
        int max_mtu;
        int min_mtu;
        struct icmp6_hdr * icmp_send;
        struct icmp6_hdr * icmp_recv;
        //struct ip6_hdr * ip_recv;
        int id;
        int sequence;
        int packet_len;
        protoent * protocol;
        int socket_id;
        char * packet_send;
        char * packet_recv;
        struct sockaddr_in6 send_sockaddr_in6;
        struct sockaddr_in6 recv_sockaddr_in6;
        timeval time;

        void make_icmp();
    enum
    {
        MIN_MTU = 150, //1280 rfc2460
        MAX_MTU = 1500,
        MAX_TTL = 30,
        TIMEOUT_S = 3
    };

};

#endif // MTU_IPV6_H
