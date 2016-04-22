/**
 * File: checksum.h
 * Author: Pavol Loffay, xloffa00@stud.fit.vutbr.cz
 * Date: 25.9.2012
 * Description: modul spracujuci checksum protolu TCP, UDP
 */

#ifndef CHECKSUM_H
#define CHECKSUM_H

#include <inttypes.h>

unsigned short checksum_calc(register int len, const unsigned short * buff);

#endif // CHECKSUM_H

