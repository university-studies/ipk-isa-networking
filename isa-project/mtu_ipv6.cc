/**
 * File: mtu_ipv6.cc
 * Author: Pavol Loffay, xloffa00@stud.fit.vutbr.cz
 * Date: 25.9.2012
 * Description: modul spracujuci checksum protolu TCP, UDP
 */

#include <iostream>
#include <cstring>
#include <errno.h>

#include "mtu_ipv6.h"
#include "checksum.h"

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/select.h>

using namespace std;

/*
 * @brief konstruktor
 *
 * @param sockaddr_in6 - struntura naplnena adresou vzdialenohe hosta atd..
 * @param int - maximalne mtu ktore sa bude testovat
 */
mtu_ipv6::mtu_ipv6(struct sockaddr_in6 h_sockaddr, int max_mtu) throw (const char *)
{
    //ak je parameter max_mtu 0, nastavi defaultne max_mtu
    if (max_mtu == 0)
        this->max_mtu = MAX_MTU;
    else 
        this->max_mtu = max_mtu;

    this->min_mtu = MIN_MTU;
    this->id = getpid();
    this->sequence = 0;
    this->packet_len = (this->min_mtu + this->max_mtu) / 2;

    send_sockaddr_in6 = h_sockaddr;
    send_sockaddr_in6.sin6_port = 0;

    /*
     * Kontrola protokolu
     */
    if ((protocol = getprotobyname("ipv6-icmp")) == NULL)
        throw "Error: icmp protocol not supported!\n";
    /*
     * Vytvorenie socketu
     */
    socket_id = socket(AF_INET6, SOCK_RAW, protocol->p_proto);
    if (socket_id < 0)
        throw "Error: socket() error!";

    /*
     * Bude ignorovat nastavene MTU ktore bolo na ceste, je to koli tomu
     * ze ked zmenim MTU tak jadro si bude stale pametat to stare mtu
     */
    int option_value = IPV6_PMTUDISC_PROBE;
    if (setsockopt(socket_id, IPPROTO_IPV6, IPV6_MTU_DISCOVER, &option_value, 
                sizeof(option_value)) != 0)
        throw "Error: setsockopt()1 error!";

    /*
     * Nastavi TTL packetu 
     *
     * TODO ci nema byt iba IPV6_HOPLIMIT
     */
    option_value = 1;
    if (setsockopt(socket_id, IPPROTO_IPV6, IPV6_RECVHOPLIMIT, &option_value, 
                sizeof(option_value)) != 0)
        throw "Error: setsockopt()2 error!";

    this->packet_send = (char *) operator new (MAX_MTU);
    this->packet_recv = (char *) operator new (MAX_MTU);
    icmp_send = (struct icmp6_hdr *)packet_send;
}

/*
 * @brief destruktor
 */
mtu_ipv6::~mtu_ipv6()
{
    close(socket_id);
    delete []packet_send;
    delete []packet_recv;
}

/*
 * @brief Metoda vytvory icmp packet
 *
 * @param void
 * @return void
 */
void mtu_ipv6::make_icmp()
{
    icmp_send->icmp6_type = ICMP6_ECHO_REQUEST;
    icmp_send->icmp6_code = 0;
    icmp_send->icmp6_id = this->id;
    icmp_send->icmp6_seq = this->sequence++;
    icmp_send->icmp6_cksum = (unsigned int) 0;
}

/*
 * @brief metoda vypocita maximalne MTU, k hostovi
 *
 * @param void
 * @return int - maximalne MTU
 */
int mtu_ipv6::calculate() throw (const char *)
{
    fd_set my_fd_set;
    bool end = false;
    bool packet_too_big = false;
    while(true)
    {
        /*
         * Vynuluje buffer 
         * Vytvori packety
         */
        memset((void *) packet_send, 0, packet_len);
        make_icmp();
        /*
         * Poslanie packetu
         */
        int bytes_send = 0;
        if (((bytes_send = sendto(socket_id, (char *)packet_send, packet_len, 0, 
                                (struct sockaddr *)&send_sockaddr_in6, 
                                sizeof(struct sockaddr_in6))) == -1) && (errno == EMSGSIZE))
        {
            //musim znizit velkost posielaneho packetu
            max_mtu = packet_len;
            packet_len = (min_mtu + max_mtu) / 2;
            continue;
        }
        //nastala ina chyba - zlihal sendto()
        else if (bytes_send == -1)
            throw "Error: sendto()!";
        /*
         * Packet sa poslal spravne
         * pockame!
         */
        time.tv_sec = TIMEOUT_S;
        time.tv_usec = 0;

        /*
         * V cykle prijma packety 
         * ak kym neprimeme ICMP_ECHOREPLY s nasim id a sequence
//1280 , najvacie 1500
//
         */
        while(true)
        {
            FD_ZERO(&my_fd_set);
            FD_SET(socket_id, &my_fd_set);
            if (select(socket_id + 1, &my_fd_set, NULL, NULL, &time) < 0)
            {
                cerr << "Error select!" << endl;
                break;
            }
          
            int bytes_recv;
            bytes_recv = 0;
            int size_sockaddr_in6 = sizeof(struct sockaddr_in6);
            if (FD_ISSET(socket_id, &my_fd_set))
            {
                /*
                 * Prijatie packetu
                 */
                bytes_recv = recvfrom(socket_id, packet_recv, MAX_MTU, 0,
                        (sockaddr *)&recv_sockaddr_in6,(socklen_t *)&size_sockaddr_in6);
                if (bytes_recv == -1)
                    throw "Error: recvfrom()!";
                
                //ip_recv = (struct ip6_hdr *) packet_recv;
                icmp_recv = (struct icmp6_hdr *)(packet_recv);
                
                /* 
                 * Test ci prijaty packet odpoveda k nasemu odoslanemu
                 */
                if ((icmp_recv->icmp6_type == ICMP6_ECHO_REPLY) && 
                        (icmp_recv->icmp6_id == id) &&
                        (icmp_recv->icmp6_seq == sequence - 1))
                {
                   //velkost packetu sa moze zvacsit
                   if (packet_too_big == true)
                   {
                       end = true;
                       break;
                   }
                    min_mtu = packet_len;
                    packet_len = (min_mtu + max_mtu) / 2;
                   //pozadovany packet prisiel mozeme zvysit velkost a poslat
                   //dalsi
                   break;
                }
                /*
                 * Test ci packet bol fragmentovany
                 */
                else if (icmp_recv->icmp6_type == ICMP6_PACKET_TOO_BIG)
                {
                    //velkost packetu musime zmensit, nas packet mal byt
                    //NASTAVIT VELKOST
                    // velkost IPv6 hlavicky je 40bytes
                    packet_len = ntohl(icmp_recv->icmp6_mtu) - 40;
                    max_mtu = packet_len;
                    packet_too_big = true;
                    break;
                }
            }
            else
            {
                //vyprsal cas ICMP packet neprisiel
                //max_mtu = packet_len;
                break;
            }

            if (end)
                break;
        }

        if (min_mtu >= max_mtu -1)
            break;
        if (end == true)
        {
            packet_len = packet_len + 40;
            break;
        }
    }

    return packet_len;
}

