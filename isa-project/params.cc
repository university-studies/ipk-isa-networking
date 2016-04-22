/**
 * File: mypmtud.cc
 * Author: Pavol Loffay, xloffa00@stud.fit.vutbr.cz
 * Date: 25.9.2012
 * Description: modul spracujuci parametre prikazoveho riadka
 */

#include "params.h"
#include "ip.h"
#include "conversion.h"

using std::string;

/*
 * @brief konstruktor nastavi vsetko na "0"
 *
 * @param void
 */
params::params()
{
    max_mtu = 0;
    param = MISSING;
    ip_or_domain = "";
}

/*
 * @brief funkcia rozparsuje parametre prikazoveho riadka
 * overi spravnost IP adries, alebo ci sa da prelozit domenove meno na IP
 *
 * @param argc
 * @param argv
 * @return void
 * @throw const char * - chybove hlasenie
 */
void params::parse(int argc, char * argv[]) throw( const char *)
{

    if (argc != PARAM_MIN && argc != PARAM_MAX)
        throw "Error: Wrong number of parameters!";

    /*
     * Bola zadana iba IP adresa
     */
    if (argc == PARAM_MIN)
        this->ip_or_domain = argv[PARAM_MIN - 1];

    /* 
     * Bola zadana IP adresa a -m max_mtu
     */
    if (argc == PARAM_MAX)
    {
        //kontrola ci bol zadany prepinac -m
        string parameter = argv[1];
        if (parameter.compare("-m") != 0)
            throw "Error: Wrong parameter [-m]!";

        //nacitanie IP
        this->ip_or_domain = argv[PARAM_MAX - 1];
        //nacitanie a overenie MTU
        if (string_to_long(string(argv[PARAM_MIN]), this->max_mtu) == false ||
                this->max_mtu < 1)
            throw "Error: Max MTU value is out of range!";

    }


    /*
     * sgment doku zisti ci posledny 
     * parameter je IPv4, IPv6 alebo domenove meno.
     */
    ipv4 ip4;
    ipv6 ip6;
    if (ip4.parse(ip_or_domain) == true)
    {
        param = IPV4;
    }
    else if (ip6.parse(ip_or_domain) == true)
    {
        param = IPV6;
    }
    else
    {
        param = DOMAIN;
    }

    param = DOMAIN;
}

/*
 * @brief funkcia vrati hodnotu mtu z prikazoveho riadka
 *
 * @param void
 * @return long hodnota MTU
 */
long params::mtu(void) const
{
    return max_mtu;
}

/*
 * @brief funkcia vrati IP adresu alebo domenove meno z prikazoveho riadka
 *
 * @param void
 * @return string IP adresa alebo domenove meno z prikazoveho riadka
 */
string params::ip_domain(void) const
{
    return ip_or_domain;
}

/*
 * @brief funkcia vrati typ parametru ktory bol zadany na prikazovom riadku, 
 * IPv4, IPv6 adresa alebo Domenove meno
 *
 * @param void
 * @return param_type_enum - typ parametru z prikazoveho riadka, IPv4, IPv6,
 *         domenove Meno
 */
params::param_type_enum params::param_type(void) const
{
    return param;
}

