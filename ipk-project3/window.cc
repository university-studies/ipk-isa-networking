/* 
 * Author: Pavol Loffay, xloffa00@stud.fit.vutbr.cz
 * Date: 14.04.2012
 * Project: Projekt 3. do predmetu IPK
 *          implementacia spolahliveho prenosu pomocou RDT
 *          pouzity protokol je UDP
 * Module: Modul spracujuci sliding window
 */

#include "window.h"

const int UNDEFINED = -5;

/*
 * @brief Funkcia inicializuje sliding window
 *
 * @param win - sliding window
 * @param size - velkost okna
 * @param void
 */
void window::init(window_item * win, int size)
{
    for (int i = 0; i < size; i++)
    {
        win[i].status = free;
        win[i].packet.set(UNDEFINED, UNDEFINED, "");
    }
}


