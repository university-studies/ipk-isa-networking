/**
 * Author: Pavol Loffay, xloffa00@stud.fit.vutbr.cz
 * Date: 14.04.2012
 * Project: Projekt 3. do predmetu IPK
 *          implementacia spolahliveho prenosu pomocou RDT
 *          pouzity protokol je UDP
 * Module: Modul implementujuci spravy posielane medzi serverom a klientom
 */

#include <stdlib.h>
#include <sstream>
#include <errno.h>
#include <limits.h>

#include "packet.h"

using std::string;

const int DECIMAL_BASE = 10;
bool string_to_long(string str, long &number);

/*
 * @brief funkcia prevedie cislo na string
 *
 * @param number - cislo ktore ma byt prevedena na string
 * @return string - retazec z cisla
 */
string int_to_string(int number)
{
    std::stringstream s_stream;
    s_stream << number;
    return s_stream.str();
}

/*
 * @brief Konstruktor
 *
 * @param id - id packetu
 * @param size - velkost datovej casti packetu
 * @param data - datova cast packetu, kodovana v base64
 */
packet_data::packet_data(long id, long size, string data)
{
    this->id_num = id;
    this->data_size = size;
    this->data_base64 = data;
}

/*
 * @brief Konstruktor, rozparsuje cely packet 
 *        korektne naplni strukturu
 *
 * @param packet - string packetu - celeho
 */
packet_data::packet_data(string packet)
{
    this->id_num = 0;
    this->data_size = 0;
    this->data_base64 = "";

    //rozparsovanie ID
    int start = packet.find(xml_tag::ID_START) + xml_tag::ID_START.length();
    if (start == string::npos)
        return;
    int end = packet.find(">", start);
    string index = packet.substr(start, end);
    string_to_long(index, this->id_num);

    //rozparsovanie SIZE
    start = packet.find(xml_tag::SIZE_START) + xml_tag::SIZE_START.length();
    if (start == string::npos)
        return;
    end = packet.find(">", start);
    string size = packet.substr(start, end);
    string_to_long(size, this->data_size);

    //rozparsovanie DATA - datova cast
    start = packet.find(xml_tag::DATA_START) + xml_tag::DATA_START.length();
    end = packet.find(xml_tag::DATA_END);
    this->data_base64 = packet.substr(start, end - start);
}

/*
 * @brief Metoda nastavi packet
 *
 * @param id - id packetu
 * @param size - velkost datovej casti packetu
 * @param data - datova cast packetu, kodovana v base64
 */
void packet_data::set(long id, long size, string data)
{
    this->id_num = id;
    this->data_size = size;
    this->data_base64 = data;
}

/*
 * @brief Metoda rozparsuje cely packet 
 *        korektne naplni strukturu
 *
 * @param packet - string packetu - celeho
 */
void packet_data::set(string packet)
{
    this->id_num = 0;
    this->data_size = 0;
    this->data_base64 = "";

    //rozparsovanie ID
    int start = packet.find(xml_tag::ID_START) + xml_tag::ID_START.length();
    if (start == string::npos)
        return;
    int end = packet.find(">", start);
    string index = packet.substr(start, end);
    string_to_long(index, this->id_num);

    //rozparsovanie SIZE
    start = packet.find(xml_tag::SIZE_START) + xml_tag::SIZE_START.length();
    if (start == string::npos)
        return;
    end = packet.find(">", start);
    string size = packet.substr(start, end);
    string_to_long(size, this->data_size);

    //rozparsovanie DATA - datova cast
    start = packet.find(xml_tag::DATA_START) + xml_tag::DATA_START.length();
    end = packet.find(xml_tag::DATA_END);
    this->data_base64 = packet.substr(start, end);
}

/*
 * @brief Metoda vrati id packetu
 *
 * @param void
 * @return int - id packetu
 */
long packet_data::id(void)
{
    return this->id_num;
}

/*
 * @brief Metoda vrati velkost datovej casti packetu
 *
 * @param void
 * @return int - velkost datovej casti packetu
 */
long packet_data::size(void)
{
    return this->data_size;
}

/*
 * @brief Metoda vrati datovu cast packetu
 *
 * @param void 
 * @return string - datova cast packetu
 */
string packet_data::data(void)
{
    return this->data_base64;
}

/*
 * @brief Metoda vytvory cely packet - obali ho do XML aby sa mohol poslat
 *
 * @param void
 * @return string - packet obaleny do XML, moze sa poslat po sieti
 */
string packet_data::str(void)
{
    string packet;
    packet = xml_tag::ROOT_START +
                xml_tag::HEADER_START + 
                    xml_tag::ID_START +
                        int_to_string(this->id_num) +
                    xml_tag::ID_END +
                    xml_tag::SIZE_START +
                        int_to_string(this->data_size) +    
                    xml_tag::SIZE_END +
                xml_tag::HEADER_END +
                xml_tag::DATA_START + 
                    this->data_base64 +
                xml_tag::DATA_END +
             xml_tag::ROOT_END;

    return packet;
}

/*
 * @brief Konstruktor
 *
 * @param id - id packetu
 */
packet_ack::packet_ack(int id)
{
    this->id_num = id;
}

/*
 * @brief Konstruktor
 *
 * @param data - cely retazec prijateho packetu
 */
packet_ack::packet_ack(string packet)
{
    //rozparsovanie ID
    int start = packet.find(xml_tag::ID_START) + xml_tag::ID_START.length();
    if (start == string::npos)
        return;
    int end = packet.find(">", start);
    string index = packet.substr(start, end);
    string_to_long(index, this->id_num);
}

/*
 * @brief Metoda vrati packet obaleni do XML, aby sa mohol poslat po sieti
 *
 * @param void
 * @return string - packet obaleny do XML - moze sa poslat po sieti
 */
string packet_ack::str(void)
{
    string ret;
    ret = xml_tag::ROOT_START +
                xml_tag::HEADER_START +
                    xml_tag::ID_START +
                        int_to_string(this->id_num) + 
                    xml_tag::ID_END + 
                xml_tag::HEADER_END +
                xml_tag::DATA_START +
                    xml_tag::ACK + 
                xml_tag::DATA_END +
          xml_tag::ROOT_END;
    return ret;
}

/*
 * @brief Metoda vrati id packetu
 *
 * @param void
 * @return id packetu
 */
long packet_ack::id(void)
{
    return this->id_num;
}


/*
 * @brief funkcia prevedie string na long a kontroluje chyby
 *
 * @param string kde je cislo
 * @param referencia kde ulozi cislo
 * @return true ak je vsetko OK, inak false;
 */
bool string_to_long(string str, long &number)
{
    char * endptr;

    number = strtol(str.c_str(), & endptr, DECIMAL_BASE);
    
    if ((errno == ERANGE && (number == LONG_MAX || number == LONG_MIN)) ||
         (errno != 0 && number == 0))
        return false;

    //no digits were found
    if (endptr == str.c_str())
        return false;

    //zacislo boli este nejake pismena
    if (*endptr != '\0')
        return false;

    return true;
}

