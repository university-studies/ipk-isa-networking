/**
 * File: convertions.cc
 * Author: Pavol Loffay, xloffa00@stud.fit.vutbr.cz
 * Date: 25.9.2012
 * Description: modul spracujuci converziu medzi typmi 
 */


#include <cstdlib>
#include <cerrno>
#include <climits>

#include "conversion.h"

using std::string;

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

