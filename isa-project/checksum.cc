/**
 * File: checksum.cc
 * Author: Pavol Loffay, xloffa00@stud.fit.vutbr.cz
 * Date: 25.9.2012
 * Description: modul spracujuci checksum protolu TCP, UDP
 */

#include "checksum.h"

#include <sys/types.h>
#include <arpa/inet.h>


/*
 * @brief funkcia na vypocet checksumu
 * @param len - dlzka spravy
 * @param buff - buffer kde je ulozena sprava
 * @return vypocitany checksum
 *
 * Funkcia je prevziata z http://www.root.cz/clanky/sokety-a-c-mtu-a-ip-fragmentace-2/
 * Autor je Radim Dostal
 */
unsigned short checksum_calc(register int len, const unsigned short *buff)
{
    register int nleft = len;
    const unsigned short *w = buff;
    register unsigned short answer;
    register int sum = 0;

    while( nleft > 1 )
    {
        sum += *w++;
        nleft -= 2;
    }

    /* mop up an odd byte, if necessary */
    if( nleft == 1 ) 
    {
        sum += htons(*(u_char *)w << 8);
    }

    /*
    * add back carry outs from top 16 bits to low 16 bits
    */

    sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
    sum += (sum >> 16);	 /* add carry */
    answer = ~sum;	 /* truncate to 16 bits */
    return (answer);
}

