/**
 * File: mypmtud.cc
 * Author: Pavol Loffay, xloffa00@stud.fit.vutbr.cz
 * Date: 25.9.2012
 * Project: Projekt do predmetu ISA, Detekcia maximalneho MTU po ceste
 *
 *  mypmtud {-m max} adresa
 *  -m max je nepovinny parameter specifikujuci hornu hranicu testovaneho MTU
 *  adresa je adresa vo formate IPv4, IPv6, alebo DNS
 *  hodnoty pre kraftovanie packetu: timeout 3 sekundy, TTL 30 hopov
 */

#include <string>
#include <iostream>
#include <cstdlib>

#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "params.h"
#include "mtu_ipv6.h"
#include "mtu_ipv4.h"

using namespace std;

const int EXCEPTION = 1;
const char * HELP_MESG = 
    "mypmtud: Prints MAX MTU size in bytes\n"
    "mypmtud {-m max MTU} address\n"
    "   -m       specify max MTU\n"
    "   address: IPv4, IPv6 or Domain name\n";

const char * RESUME =
    "resume: ";
const char * BYTES =
    " bytes";

/*
 * @brief funkcia vytlaci error a ukonci program
 */
void print_error_exit(const char * err)
{
    std::cerr << err << std::endl;
    throw EXCEPTION;
}

/*
 * @brief funkcia prelozi domenove meno
 *  a z sistenych adries z getaddrinfo vypise maximalne MTU
 * @param string - domenove meno alebo ip adresa verize 4 a 6
 * @param mtu - maximalne mtu do ktoreho sa ma hladat
 * @return najdene mtu
 */
int max_mtu(string domain, int mtu)
{
    /*
     * premenne na ukladanie vypocitaneho mtu
     * a vypis ziskanej adresy
     */
    int max_mtu = 0;
    int actual_mtu = 0; 
    char addrstr[100] = {0};

    //nastave nie struktury addrinfo, aby getaddrinfo ziskal adresy
    struct addrinfo addrinfo_struct, *res;
    memset (&addrinfo_struct, 0, sizeof (struct addrinfo));
    addrinfo_struct.ai_family = PF_UNSPEC;
    addrinfo_struct.ai_socktype = SOCK_STREAM;
    addrinfo_struct.ai_flags |= AI_CANONNAME;

    // stuktury pre popis adresy vzdialeneho hosta, IPv6, IPv4
    struct sockaddr_in6 host_6;
    struct sockaddr_in host_4;

    if (getaddrinfo (domain.c_str(), NULL, &addrinfo_struct, &res) != 0)
        return -1;

    while (res)
    {
        /*
        * prevedie IP z binarky do textu
        * inet_ntop(Adress_family, source, dest, size);
        */
        switch (res->ai_family)
        {
            case AF_INET:
            {
                memcpy(&host_4, res->ai_addr, sizeof(struct sockaddr_in));
                inet_ntop (res->ai_family, &host_4.sin_addr.s_addr, addrstr, 100);

                mtu_ipv4 mtu4(host_4, mtu);
                actual_mtu = mtu4.calculate();
                if (actual_mtu > max_mtu)
                    max_mtu = actual_mtu;
                
                break;
            }

            case AF_INET6:
            {
                memcpy(&host_6, res->ai_addr, sizeof(struct sockaddr_in6));
                inet_ntop (res->ai_family, &host_6.sin6_addr, addrstr, 100);

                mtu_ipv6 mtu6(host_6, mtu);
                actual_mtu = mtu6.calculate();
                if (actual_mtu > max_mtu)
                    max_mtu = actual_mtu;

                break;
            }
            default:
            {
            }
        }

        memset(addrstr, 0, 100);
        res = res->ai_next;
    }

  return max_mtu;
}

/*
 * Funkcia main
 */
int main(int argc, char * argv[])
{
    /*
     * Kontrola uid, musi to byt root - UID = 0
     */
    if (getuid() != 0)
    {
        std::cerr << "Error: run as a root!\n";
        return EXIT_FAILURE;
    }

    /*
     * Spracovanie parametrov
     */
    params param;
    try
    {
        param.parse(argc, argv);
    }
    catch (const char * msg)
    {
        std::cerr << msg << std::endl;
        std::cerr << std::endl;
        std::cerr << HELP_MESG;
        return EXIT_FAILURE;
    }

    try
    {
        int mtu = max_mtu(param.ip_domain(), param.mtu());
        if (mtu != -1)
            cout << RESUME << mtu << BYTES << endl;
        else
        {
            //ak to bola IPv4 a nepodaril sa preklad domenoveho mena
            if (param.param_type() == params::IPV4)
            {
                struct sockaddr_in host;
                host.sin_family = AF_INET;
                host.sin_port = 0;
                inet_pton(AF_INET, param.ip_domain().c_str(), &host.sin_addr);

                mtu_ipv4 mtu4(host, param.mtu());
                mtu = mtu4.calculate();
                cout << RESUME << mtu << BYTES << endl;
            }
            //ak to bola IPv6 a nepodaril sa preklad domenoveho mena
            else if (param.param_type() == params::IPV4)
            {
                struct sockaddr_in6 host;
                host.sin6_family = AF_INET6;
                host.sin6_port = 0;
                inet_pton(AF_INET6, param.ip_domain().c_str(), &host.sin6_addr);

                mtu_ipv6 mtu6(host, param.mtu());
                mtu = mtu6.calculate();
                cout << RESUME << mtu << BYTES << endl;
            }
            cerr << "Error: Couldn't find host!" << endl;
            return EXIT_FAILURE;
        }

    }
    catch (const char *msg)
    {
        cerr << msg << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

