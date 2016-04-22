/* 
 * Author: Pavol Loffay, xloffa00@stud.fit.vutbr.cz
 * Date: 14.04.2012
 * Project: Projekt 3. do predmetu IPK
 *          implementacia spolahliveho prenosu pomocou RDT
 *          pouzity protokol je UDP
 * Module: Modul spracujuci sliding window
 */

#ifndef WINDOW_H_
#define WINDOW_H_

#include "packet.h"

namespace window
{
    /*
     * Velkost sliding window
     */
    const int SIZE_CLIENT = 10; 

    /*
     * stavy packetu
     */
    enum state
    {
        free,
        to_send,
        send,
        ack,
        writed
    };

    /*
     * Struktura pre polozku sliding window
     */
    struct window_item
    {
        packet_data packet;
        state status;
    };

    void init(window_item * win, int size);
}

#endif /* WINDOW_H_ */

