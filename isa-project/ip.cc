/**
 * File: ip.cc
 * Author: Pavol Loffay, xloffa00@stud.fit.vutbr.cz
 * Date: 25.9.2012
 * Description: modul spracujuci ip adresy 
 */

#include <iostream>

#include "ip.h"
#include "conversion.h"

using std::string;

/*
 * @brief konstruktor pre ipv4, vynuluje polozky pola ipv4
 *
 * @param void
 */
ipv4::ipv4(void)
{
    for (int i = 0; i < 4; i++)
        ip[i] = 0;
}

/*
 * @brief funkcia otestuje cast ipv4, jednu stvoricu
 *
 * @param long jedna stvorica
 * @return true, ak to moze byt cislo pre ipv4, false ak nie
 */
bool ipv4::test(long number)
{
    if (number > 255)
        return false;

    if (number < 0)
        return false;

    return true;
}

/*
 * @brief funkcia vypise ipv4
 *
 * @param void
 * @return void
 */
void ipv4::print(void) const
{
    for (int i = 0; i < 4; i++)
    {
        if (i == 3)
            std::cout << ip[i] << "\n";
        else
            std::cout << ip[i] << ".";
    }
}

/*
 * @brief funkcia vrati celu ip adresu
 *
 * @param void
 * @return string cela ip adresa
 */
string ipv4::ret(void) const
{
    string ret;

    for (int i = 0; i < IPV4_BLOCKS; i++)
    {
        if (i != IPV4_BLOCKS - 1)
            ret.append(to_string(ip[i]) + ".");
        else
            ret.append(to_string(ip[i]));

    }

    return ret;
}

/*
 * @brief funkcia rozparsuje string a ulozi ho do struktury ipv4
 *
 * @param string kde je ipv4
 * @return true, ak string obsahoval ipv4, false ak neobsahoval
 */
bool ipv4::parse(string str)
{
    //cyklus prejde 3x, stvrty krat nenajde bodku ale je tam cislo
    int i = 0;
    for (i = 0; i < 3; i++)
    {
        string number_str = str.substr(0, str.find_first_of("."));
        if (string_to_long(number_str, ip[i]) == false)
            return false;

        //kontrola rozsahu cisla z ip
        if (test(ip[i]) == false)
            return false;

        str.erase(0, str.find_first_of(".") + 1);
    }

    /*
     * Spracovanie posledneho cisla
     */
    if (string_to_long(str, ip[i]) == false)
        return false;

    if (test(ip[i]) == false)
        return false;

    return true;
}


/*
 * @brief defaultny konstruktor, vynuluje vsetko 
 *
 * @param void
 */
ipv6::ipv6(void)
{
    ip = "";
}

/*
 * @brief funkcia skontroluje cast ipv6 adresy
 *
 * @param string cast ipv6 adresy
 * @return bool, true ak splna specifikaciu ipv6, false ak nie
 */
bool ipv6::test(string ip_part)
{
    //kontrola dlzky ipv6 bloku
    if (ip_part.length() > IPV6_BLOCK_LENGTH)
        return false;

    //kontrola ci obsahuje znaky 0-9,a-f
    for (unsigned int i = 0; i < ip_part.length(); i++)
    {
        if (! ((ip_part[i] >= '0' && ip_part[i] <= '9') || 
            (ip_part[i] >= 'a' && ip_part[i] <= 'e') || 
            (ip_part[i] >= 'A' && ip_part[i] <= 'E')))
            return false;
    }

    return true;
}

/*
 * @brief funkcia vrati celu ipv6
 *
 * @param void
 * @return string ip adresa
 */
string ipv6::ret(void) const
{
    return ip;
}

/*
 * @brief funkcia rozparsuje ipv6
 *
 * @param string, ktory obsahuje ipv6 adresu
 * @return bool, true ak je to ipv6 adresa, false ak nie
 */
bool ipv6::parse(string str)
{
    //zaloha ip_adresy
    string back_up_str = str;

    /*
     * Kontrola ci je ipv6 v spravnom skratenom tvare
     * cize neobsahuje dve casti s ::, napr ::1::2 je zla adresa
     */
    string short_ip_test = str;
    if (short_ip_test.find("::") != string::npos)
    {
        short_ip_test.erase(0, short_ip_test.find("::") + string("::").length());
        if (short_ip_test.find("::") != string::npos)
            return false;
    }

    /*
     * cyklus najde v adrese znak ":" a odstrani 
     * z adresy cast od 0 do znaku ":" a tu cast aj skontroluje
     * takto prejde celu adresu
     */
    int index = 0;
    while (str.length() > 0)
    {
        //kontrola ci nebola zadana dlhsia IP adresa
        if (index == 8)
            return false;

        //ziskanie casti ipv6 oddeleneho dvojbodkou :
        string ip_part = str.substr(0, str.find_first_of(":"));

        //kontrola casti ipv6 :2001:
        if (test(ip_part) == false)
            return false;

        if (str.find_first_of(":") == string::npos)
            str.erase(0, str.length());
        else
            str.erase(0, str.find_first_of(":") + 1);

        index++;
    }

    ip = back_up_str;

    return true;
}

