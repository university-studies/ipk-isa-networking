/**
 * Author: Pavol Loffay, xloffa00@stud.fit.vutbr.cz
 * Date: 14.04.2012
 * Project: Projekt 3. do predmetu IPK
 *          implementacia spolahliveho prenosu pomocou RDT
 *          pouzity protokol je UDP
 * Module: Modul pre server - prima data
 *
 * evaluation: 100%
 */

#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <fcntl.h>
#include <cstdio>
#include <string.h>

#include "params.h"
#include "udt.h"
#include "base64.h"
#include "packet.h"
#include "window.h"

using std::cerr;
using std::endl;
using std::string;
using std::cout;

/*
 * pole kde sa ulozia data zo socketu
 */
const int DATA_IN_MAX = 1000;
char * data_in; 

/*
 * Sliding window
 */
window::window_item s_window[window::SIZE_CLIENT];

/*
 * Localhost IP
 */
const in_addr_t LOCALHOST = 0x7f000001;

/*
 * Help sprava
 */
const char * HELP_MESG = 
    "rdtserver - prijimatel dat\n"
    "rdtserver -s source_port -d destination_port\n";

/*
 * Funkcia zachyti signal - SIGTERM
 */
void signal_catcher(int)
{
    delete []data_in;
    exit(0);
}

/*
 * @brief Funkcia posle ACK packet s danym id
 *
 * @param  udt - cislo socketu
 * @param port - cislo portu
 * @param id - id ack packetu
 * @return bool - true ak nedoslo k chybe, inak false
 */
bool send_ack(int udt, long port, long id)
{
    packet_ack ack(id);

    if (udt_send(udt, LOCALHOST, port, (char *)ack.str().c_str(),
                 ack.str().length()) != 1)
        return false;

    return true;
}

/*
 * @brief Funkcia pre server
 * 
 * @param source_p - port na ktorom sa primaju data
 * @param dest_p port na ktory sa posielaju ACK
 * @return true, ak nedoslo k chybe, inak falsle
 */
bool server(long source_p, long dest_p)
{
    data_in = new char[DATA_IN_MAX];
    freopen(NULL, "wb", stdout);
    window::init(s_window, window::SIZE_CLIENT);

    /*
     * Inicializacia
     */
    int udt = udt_init(source_p);
    
    /*
     * Prijimanie dat
     */
    fd_set readfds;
    FD_ZERO(&readfds);          //clear a set
    FD_SET(udt, &readfds);      //add file descriptor to a set
    FD_SET(STDIN_FILENO, &readfds); //dopnit ?
    string buffer;
    long rcv_base = 0;
    while(select(udt + 1, &readfds, NULL, NULL, NULL))
    {
        memset(data_in, 0, DATA_IN_MAX);
        udt_recv(udt, data_in, DATA_IN_MAX - 1, NULL, NULL);
     
        buffer.append(string(data_in));

        //cerr << "\t\t\tSERVER data = " << data_in << endl;

        /*
         * Naplnenie slide window
         *  - posielanie ack
         */
        while (buffer.find(xml_tag::ROOT_START) != std::string::npos && 
               buffer.find(xml_tag::ROOT_END) != std::string::npos)
        {
            //rozparsovanie packetu
            int start = buffer.find(xml_tag::ROOT_START);
            int end = buffer.find(xml_tag::ROOT_END) + 
                        xml_tag::ROOT_END.length();
            
            //vytvory sa packet - ktory prisiel - aby sa rozparsoval
            packet_data packet(buffer.substr(start, 
                        end - start + xml_tag::ROOT_END.length()));
            
            //cerr << "\t\t\tSERVER base = " << packet.id() << endl;

            /*
             * Posle ACK
             * n in <rcv_base - N, rcv_base - 1>
             */
            if (packet.id() >= rcv_base - window::SIZE_CLIENT && 
                packet.id() <= rcv_base -1)
            {
                //cerr << "\t\t\tSERVER ack base = " <<packet.id() << endl; 
                //poslanie ack
                if (! send_ack(udt, dest_p, packet.id()))
                    return false;
            }
            /*
             * Posle ACK
             * ulozi do buffera, vypise ak je prvy
             * n in <rcv_base, rcv_base + N - 1>
             */
            else if (packet.id() >= rcv_base && 
                     packet.id() <= rcv_base + window::SIZE_CLIENT - 1)
            {
                //cerr << "\t\t\tSERVER ack + write? base = " << packet.id() << endl;
                //poslanie ack
                if (! send_ack(udt, dest_p, packet.id()))
                    return false;
            
                //ulozenie do sliding window
                s_window[packet.id() % window::SIZE_CLIENT].packet = packet;

                //vypis packetu
                while (s_window[rcv_base % window::SIZE_CLIENT].packet.id() == rcv_base)
                {
                    //cerr << "\t\t\tSERVER > writing_base = " << 
                        //s_window[rcv_base % window::SIZE_CLIENT].packet.id()<< endl;
                        
                    fwrite(base64_decode(s_window[rcv_base % window::SIZE_CLIENT].packet.data()).c_str(), 1,
                                         s_window[rcv_base % window::SIZE_CLIENT].packet.size(), stdout);
                    fflush(stdout);
                    rcv_base++;
                }
            }
            /*
             * Packet bude zahodeny
             * n in (-inf, rcv_base - N - 1> or <rcv_base + N, inf)
             * N - je velkost okna
             */

            //vymaze sa z buffera
            buffer.erase(start, end - start + xml_tag::ROOT_END.length());

        }
    }
    return true;
}

/*
 * Main
 */
int main(int argc, char *argv[])
{
    //zacitavanie signalov
    signal(SIGTERM, signal_catcher);

    //spracovanie parametrov
    params param;
    try
    {
        param.parse(argc, argv);
    }
    catch(param_exception &e)
    {
        cerr << HELP_MESG;
        //popis chyby
        cerr << e.err() << endl;
        return EXIT_FAILURE;
    }

    if (! server(param.source(), param.dest()))
    {
        cerr << "Chyba pri komunikacii" << endl;
        delete []data_in;
        return EXIT_FAILURE;
    }

    delete []data_in;
    return EXIT_SUCCESS;
}

