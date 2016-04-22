/**
 * Author: Pavol Loffay, xloffa00@stud.fit.vutbr.cz
 * Date: 14.04.2012
 * Project: Projekt 3. do predmetu IPK
 *          implementacia spolahliveho prenosu pomocou RDT
 *          pouzity protokol je UDP
 * Module: Modul spracujuci parametre prikazoveho riadka
 */

#include <cstdlib>
#include <cerrno>
#include <climits>

#include "params.h"
#include <iostream>

using std::string;

/*
 * @brief Konstruktor
 */
params::params(void)
{
    source_port = 0;
    dest_port = 0;
}

/*
 * @brief Metoda rozparsuje parametre prikazoveho riadka
 *
 * @param argc - ukazuje za posledny parameter prik. riadka
 * @param argv - parametre
 * @return void
 */
void params::parse(int argc, char * argv[]) throw(param_exception)
{
    //argc ukazuje vzdy za parametre cize + 1 
    if (argc != PARAM_NUMBER + 1)
        throw param_exception("Params: nespravny pocet parametrov - ");

    //argv[0] = ./program_name
    //index prveho parametra v argv[index]
    int index = 1;
    while (index < argc)
    {
        string param = argv[index];
        if (param.compare("-s") == 0)
        {
            //source port
            if (! string_to_long(string(argv[index + 1]), this->source_port))
                throw param_exception("Params: nespravny source_port");

            index = index + 2;
        }
        else if (param.compare("-d") == 0)
        {
            //dest port
            if (! string_to_long(string(argv[index + 1]), this->dest_port))
                throw param_exception("Params: nespravny dest_port");

            index = index + 2;
        }
    }
}

/*
 * @brief Metoda vrati cislo destination portu
 *
 * @param void 
 * @param long - cislo destination portu
 */
long params::dest(void) const
{
    return this->dest_port;
}

/*
 * @brief Metoda vrati cislo source portu
 *
 * @param void
 * @param long - cislo source portu
 */
long params::source(void) const
{
    return this->source_port;
}

/*
 * @brief funkcia prevedie string na long a kontroluje chyby
 *
 * @param string kde je cislo
 * @param referencia kde ulozi cislo
 * @return true ak je vsetko OK, inak false;
 */
bool params::string_to_long(string str, long &number)
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

/*
 * @brief Konstruktor pre triedu, ktora sa pouziva ako vynimka
 *
 * @param err - popis chyby
 */
param_exception::param_exception(string err)
{
    this->mesg = err;
}

/*
 * @brief metoda ktora vrati popis chyby
 *
 * @param void
 * @return popis chyby 
 */
string param_exception::err(void)
{
    return this->mesg;
}

