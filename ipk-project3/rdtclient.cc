/**
 * Author: Pavol Loffay, xloffa00@stud.fit.vutbr.cz
 * Date: 14.04.2012
 * Project: Projekt 3. do predmetu IPK
 *          implementacia spolahliveho prenosu pomocou RDT
 *          pouzity protokol je UDP
 * Module: Modul pre klienta - posiela data
 */


#include <iostream>
#include <cstdlib>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>

#include <stdio.h>

#include "base64.h"
#include "params.h"
#include "udt.h"
#include "packet.h"
#include "window.h"

using std::cerr;
using std::endl;
using std::string;

/*
 * Casovac
 * doba cakania v sekundach
 */
struct itimerval itv;
sigset_t sigmask;
const int TIMEOUT_SEC = 5;

/*
 * Localhost IP
 */
const in_addr_t LOCALHOST = 0x7f000001;

/*
 * maximalna velkost udp packetu je 512B 
 * BUFFER_MAX udava maximalnu velkost dat v data casti
 */
const int DATA_IN_MAX = 200;
const int DATA_ACK_MAX = 600;

/*
 * Sliding window
 */
window::window_item s_window[window::SIZE_CLIENT];

/*
 * Source, destination port z prik. riadka
 * udt - cislo socketu po udt_inot()
 */
long destination_port;
long source_port;
int udt;

/*
 * Help sprava
 */
const char * HELP_MESG = 
    "rdtclient - odosielatel dat\n"
    "rdtclient -s source_port -d destination_port\n";

/*
 * @brief Funkcia spusti casovac
 *
 * @param void 
 * @return void
 */
void timer_start(void)
{
    //sigemptyset(&sigmask);
    //sigaddset(&sigmask, SIGALRM);
    //cerr << " timer";

    setitimer(ITIMER_REAL, &itv, NULL);
    sigprocmask(SIG_UNBLOCK, &sigmask, NULL);

    return;
}

/*
 * @brief Funkcia zastavi casovac
 *
 * @param void
 * @return void
 */
void timer_stop(void)
{
    sigprocmask(SIG_BLOCK, &sigmask, NULL);

    return;
}
/*
 * @brief Obsluzna funkcia pri vyprsani casovaca
 *
 * @param int -
 * @return void
 */
void signal_alarm(int)
{
    //cerr << "CLIENT > resending" << endl;  

    bool start = false;

    //znovu odoslane nepotvrdenych dat
    for (int i = 0; i < window::SIZE_CLIENT; i++)
    {
        //status je taky ze bol poslany, ale neprislo ack
        if (s_window[i].status == window::send)
        {
            start = true;

            //cerr << "CLIENT > resend id = " << s_window[i].packet.id() <<endl;
            if (udt_send(udt, LOCALHOST, destination_port, 
                     (char *)s_window[i].packet.str().c_str(),
                      s_window[i].packet.str().length()) != 1)
            {
                cerr << "Chyba pri odosielani dat!" << endl;
                exit(EXIT_FAILURE);
            }
        }
    }
    if (start)
        timer_start();
    
    //znovu nastavenie obsluhy
    signal(SIGALRM, signal_alarm);

    return;
}

/*
 * @brief Funkcia pre komunikaciu klienta
 *
 * @param source_p - zdrojovy port, kde sa primaju ack
 * @param dest_p - cielovy port, kam sa posielaju data
 * @return false, ak doslo k chybe, inak true
 */
bool client(void)
{
    int fread_ret;
    char * data_in = new char[DATA_IN_MAX];
    char * data_ack = new char[DATA_ACK_MAX];
    freopen(NULL, "rb", stdin);
    //inicializacia slide window
    window::init(s_window, window::SIZE_CLIENT);

    /*
     * Inicializacia spojenia
     */
    udt = udt_init(source_port);

    /*
     * Posielanie dat
     */
    fd_set readfds;
    FD_ZERO(&readfds);          //clear a set
    FD_SET(udt, &readfds);      
    FD_SET(STDIN_FILENO, &readfds); //add file descriptor to a set
    long send_base  = 0;
    long next_seqnum = 0;

    string buffer_ack;
    while(select(udt + 1, &readfds, NULL, NULL, NULL))
    {

        /*
         * Naplnenie okna
         */
        bool timer;
        timer = false;
        while (next_seqnum < send_base + (window::SIZE_CLIENT - 1) && !feof(stdin))
        {
            timer = true;

            memset(data_in, 0, DATA_IN_MAX);
            fread_ret = fread(data_in, 1, DATA_IN_MAX - 1, stdin);
            string data_coded = base64_encode(
                 reinterpret_cast<const unsigned char *>(data_in),
                 fread_ret);


            s_window[next_seqnum % window::SIZE_CLIENT].packet.set(next_seqnum,
                                                        fread_ret, data_coded);

            //cerr << "CLIENT > send " << next_seqnum <<endl;
            /*
             * poslanie packetu
             */
            if (udt_send(udt, LOCALHOST, destination_port,
                        (char *)s_window[next_seqnum % window::SIZE_CLIENT].packet.str().c_str(),
                        s_window[next_seqnum % window::SIZE_CLIENT].packet.str().length()) != 1)
            {
                delete []data_in;
                delete []data_ack;
                return false;
            }

            s_window[next_seqnum % window::SIZE_CLIENT].status = window::send;
            next_seqnum++;
        }
        if (timer)
            timer_start();
        
        /*
         * Primanie ack
         */
        if (FD_ISSET(udt, &readfds))
        {
            memset(data_ack, 0, DATA_ACK_MAX);
            
            udt_recv(udt, data_ack, DATA_ACK_MAX - 1, NULL, NULL);
            buffer_ack.append(string(data_ack));

            /*
             * zisti ci je tam cely packet
             */
            while (buffer_ack.find(xml_tag::ROOT_START) != string::npos &&
                   buffer_ack.find(xml_tag::ROOT_END) != string::npos)
            {
                int start = buffer_ack.find(xml_tag::ROOT_START);
                int end = buffer_ack.find(xml_tag::ROOT_END) + 
                            xml_tag::ROOT_END.length();

                string one_packet = buffer_ack.substr(start, end - start +
                                                        xml_tag::ROOT_END.length());

                packet_ack ack(one_packet);
               //cerr << "CLIENT > ack = " << ack.id() << endl;

                /*
                 * oznacennie ACK
                 */
                s_window[ack.id() % window::SIZE_CLIENT].status = window::ack;

                //oznaci packety za prijate a posunie okno
                while(s_window[send_base % window::SIZE_CLIENT].status == window::ack)
                {
                    send_base++;
                    if (send_base == next_seqnum)
                        break;
                }

                buffer_ack.erase(start, end - start + xml_tag::ROOT_END.length());
            }
        }

        /*
         * ukoncit?
         */
        if (feof(stdin))
        {
            bool end;
            end = true;
            for (int i = 0; i < window::SIZE_CLIENT; i++)
            {
                if (s_window[i].status == window::send)
                {
                    end = false;
                    break;
                }
            }
            if (end)
            {
                break;
                timer_stop();
                return true;
            }
        }

        FD_ZERO(&readfds);          
        FD_SET(udt, &readfds);      
        FD_SET(STDIN_FILENO, &readfds);
    }

    //cerr << "CLIENT end" << endl;
    delete []data_in;
    delete []data_ack;

    return true;
}

/*
 * Main
 */
int main(int argc, char *argv[])
{
    signal(SIGALRM, signal_alarm);
    itv.it_interval.tv_sec = TIMEOUT_SEC;
    itv.it_interval.tv_usec = 0;
    itv.it_value.tv_sec = TIMEOUT_SEC;
    itv.it_value.tv_usec = 0;


    //spracovanie parametrov
    params param;
    try
    {
        param.parse(argc, argv);
    }
    catch(param_exception &e)
    {
        std::cerr << HELP_MESG;
        // popis chyby
        std::cerr << e.err() << endl;
        return EXIT_FAILURE;
    }

    source_port = param.source();
    destination_port = param.dest();
    if (!client())
    {
        //chyba pri komunikacii
        std::cerr << "Chyba pri komnikacii!";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

