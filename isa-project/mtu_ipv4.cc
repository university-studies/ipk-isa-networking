/**
 * File: mtu_ipv4.cc
 * Author: Pavol Loffay, xloffa00@stud.fit.vutbr.cz
 * Date: 25.9.2012
 * Description: modul implementujuci zistenie MTU pre IPv4 za pomoci
 *  DF bitu - Don't Fragment - ked je nastaveny uzol nefragmentuje spravu
 */

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <errno.h>

#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>

#include "checksum.h"
#include "mtu_ipv4.h"


using namespace std;

/*
 * @brief Konstruktor, zisti dostupnost ICMP portokolu
 *
 * @param sockaddr_in - struktura naplnena adresou vzdialeneho hosta atd..
 * @param int - maximalne mtu
 * @throw const char * - chybove hlasenie 
 */
mtu_ipv4::mtu_ipv4(struct sockaddr_in h_sockaddr, int max_mtu) throw (const char *)
{
    //nastavenie MTU
    if (max_mtu == 0)
        this->max_mtu = MAX_MTU;
    else
        this->max_mtu = max_mtu;

    this->min_mtu = MIN_MTU;
    this->id = getpid();
    this->sequence = 0;
    this->packet_len = (this->min_mtu + this->max_mtu) / 2;
    send_sockaddr = h_sockaddr;
    send_sockaddr.sin_port = 0;
    
    /*
     * Kontrola protokolu
     */
    if ((protocol = getprotobyname("icmp")) == NULL)
        throw "Error: icmp protocol not supported!\n";

    /*
     * Vytvorenie socketu
     */
    socket_id = socket(AF_INET, SOCK_RAW, protocol->p_proto);
    if (socket_id < 0)
        throw "Error: socket() error!";

    /* 
     *  IP_HDRINCL musi byt nastavene aby kernel
     *  automaticky nepridaval IP hlavicku
     */
    int option_value = 1; //1 podla root s 0 to neslo miesto SOL_IP ma IPPROTO_ICMP
    //alebo IPPROTO_IP
    if (setsockopt(socket_id, SOL_IP, IP_HDRINCL, &option_value, 
                sizeof(option_value)) != 0)
        throw "Error: setsockopt()1 error!";
    //toto tiez nastavit pre  ipv6 tiez

    /*
     * Bude ignorovat nastavene MTU ktore bolo na ceste, je to koli tomu
     * ze ked zmenim MTU tak jadro si bude stale pametat to stare mtu
     */
    option_value = IP_PMTUDISC_PROBE;
    if (setsockopt(socket_id, IPPROTO_IP, IP_MTU_DISCOVER, &option_value, 
                sizeof(option_value)) != 0)
        throw "Error: setsockopt()2 error!";

    /*
     * Nastavi TTL packetu
     */
    option_value = MAX_TTL;
    if (setsockopt(socket_id, IPPROTO_IP, IP_TTL, &option_value, 
                sizeof(option_value)) != 0)
        throw "Error: setsockopt()3 error!";

    this->recv_packet = (char *) operator new (MAX_MTU);
    this->send_packet = (char *) operator new (MAX_MTU);
    ip_send = (struct iphdr *)send_packet;
    icmp_send = (struct icmphdr *)((char *) ip_send + sizeof(struct iphdr));
}

/*
 * @brief destruktor
 */
mtu_ipv4::~mtu_ipv4()
{
    close(socket_id);
    delete[] recv_packet;
    delete[]  send_packet;
}

/*
 * Vypocita MTU
 */
int mtu_ipv4::calculate(void) throw (const char *)
{
    fd_set my_fd_set;
    while(true)
    {
        /*
         * Vynuluje buffer 
         * Vytvori packety
         */
        memset((void *) ip_send, 0, packet_len);
        make_icmp();
        make_ip();
        /*
         * Poslanie packetu
         */
        int bytes_send = 0;
        if (((bytes_send = sendto(socket_id, (char *)ip_send, packet_len, 0, 
                                (struct sockaddr *)&send_sockaddr, 
                                sizeof(struct sockaddr))) == -1) && (errno == EMSGSIZE))
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
            int size_sockaddr_in = sizeof(struct sockaddr_in);
            if (FD_ISSET(socket_id, &my_fd_set))
            {
                /*
                 * Prijatie packetu
                 */
                bytes_recv = recvfrom(socket_id, recv_packet, MAX_MTU, 0,
                        (sockaddr *)&recv_sockaddr,(socklen_t *)&size_sockaddr_in);
                if (bytes_recv == -1)
                    throw "Error: recvfrom()!";
                
                ip_recv = (struct iphdr *) recv_packet;
                icmp_recv = (struct icmphdr *)(recv_packet + ip_recv->ihl * 4);
                
                /* 
                 * Test ci prijaty packet odpoveda k nasemu odoslanemu
                 */
                if ((icmp_recv->type == ICMP_ECHOREPLY) && 
                        (icmp_recv->un.echo.id == id) &&
                        (icmp_recv->un.echo.sequence == sequence - 1))
                {
                   //velkost packetu sa moze zvacsit
                   min_mtu = packet_len;

                   //pozadovany packet prisiel mozeme zvysit velkost a poslat
                   //dalsi
                   break;
                }
                /*
                 * Test ci packet bol fragmentovany
                 */
                else if (fragment_need(bytes_recv - ip_recv->ihl * 4))
                {
                    //velkost packetu musime zmensit, nas packet mal byt
                    //fragmetovany
                    //NASTAVIT VELKOST
                    max_mtu = packet_len;
                    break;
                }
            }
            else
            {
                //vyprsal cas ICMP packet neprisiel
                max_mtu = packet_len;
                break;
            }
        }

        packet_len = (min_mtu + max_mtu) / 2;
        if (min_mtu >= max_mtu -1)
            break;
    }

    return packet_len;
}

/*
 * @brief Metoda ak prijaty ICMP odoslane ECHO bolo 
 *  bolo zahodene a packet musi byt fragmentovany
 *
 *  @param void
 *  @return void
 */
bool mtu_ipv4::fragment_need(unsigned int len)
{
    /*
     * prijaty icmp packet obsahuje stary icmp packet a ip packet
     */
    if (len < 2 * sizeof(struct icmphdr) + sizeof(iphdr))
        return false;

   struct iphdr *ip_old = (struct iphdr *)(recv_packet + sizeof(icmphdr));
   struct icmphdr *icmp_old = (struct icmphdr*)((char *)ip_old + ip_old->ihl * 4);
   if (len < 2 * sizeof(icmphdr) + ip_send->ihl * 4)
     return false;

   if ((icmp_recv->type == ICMP_DEST_UNREACH) && 
        (icmp_recv->code == ICMP_FRAG_NEEDED) && 
        (icmp_old->type == ICMP_ECHO) && 
        (icmp_old->code == 0) && 
        (icmp_old->un.echo.id == id) && 
        (icmp_old->un.echo.sequence == sequence - 1)) 
        return true;

   return false;
}

/*
 * @brief Metoda vytvory icmp packet
 *
 * @param void
 * @return void
 */
void mtu_ipv4::make_icmp()
{
    icmp_send->type = ICMP_ECHO;
    icmp_send->code = 0;
    icmp_send->un.echo.id = id;
    icmp_send->un.echo.sequence = this->sequence++;
    icmp_send->checksum = (unsigned int) 0;
    //zahrna aj data ktore nesie ICMP sprava //TODO mal som - icmphdr
    icmp_send->checksum = checksum_calc(this->packet_len - sizeof(struct iphdr),
                                          (unsigned short *) icmp_send);
}

/*
 * @brief Metoda vytvory IPv4 packet
 *
 * @param void
 * @return void
 */
void mtu_ipv4::make_ip()
{
    ip_send->ihl = 5;
    ip_send->version = 4; 
    ip_send->tos = 0;
    ip_send->tot_len = htons(packet_len);
    //doplni jadro OS
    ip_send->id = 0;
    ip_send->ttl = MAX_TTL;
    ip_send->frag_off = htons(DF);
    ip_send->protocol = IPPROTO_ICMP;
    //doplni jadro OS
    ip_send->check = 0;
    //doplni jadro OS
    ip_send->saddr = 0; 
    ip_send->daddr = *((unsigned long int*) &send_sockaddr.sin_addr.s_addr);
}

