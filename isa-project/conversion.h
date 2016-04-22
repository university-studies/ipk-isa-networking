/**
 * File: convertions.cc
 * Author: Pavol Loffay, xloffa00@stud.fit.vutbr.cz
 * Date: 25.9.2012
 * Description: modul spracujuci converziu medzi typmi 
 */

#ifndef CONVERSION_H
#define CONVERSION_H

#include <string>
#include <sstream>

const int DECIMAL_BASE = 10;
bool string_to_long(std::string str, long &number);

template <class T>
inline std::string to_string(const T& type)
{
    std::stringstream s_stream;
    s_stream << type;
    return s_stream.str();
}

#endif // CONVERTIONS_H

