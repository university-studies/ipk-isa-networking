/**
 * Author: Pavol Loffay, xloffa00@stud.fit.vutbr.cz
 * Date: 14.04.2012
 * Project: Projekt 3. do predmetu IPK
 *          implementacia spolahliveho prenosu pomocou RDT
 *          pouzity protokol je UDP
 * Module: Modul implementujuci spravy posielane medzu serverom a klientom
 */

#ifndef PACKET_H_
#define PACKET_H_

#include <string>

/*
 * Retazce z ktorych sa skladaju posielane spravy
 * -casti xml tagov
 */
namespace xml_tag
{
    const std::string ROOT_START("<rdt-segment id=\"xloffa00\">");
    const std::string ROOT_END("</rdt-segment>");
    const std::string HEADER_START("<header>");
    const std::string HEADER_END("</header>");
    const std::string DATA_START("<data>");
    const std::string DATA_END("</data>");
    const std::string ID_START("<packet_id id=\"");
    const std::string ID_END("\">");
    const std::string SIZE_START("<data_size size=\"");
    const std::string SIZE_END("\">");
    const std::string ACK("ACK");
}

/*
 * @brief trieda charakterizujuca packet ktory posiela
 *        client s datami
 */
class packet_data
{
    public:
        packet_data() {};
        packet_data(long id, long size, std::string data);
        packet_data(std::string packet);

        void set(long id, long size, std::string data);
        void set(std::string packet);
        //vrati id packetu
        long id(void);
        //vrati velkost data casti packetu
        long size(void);
        //vrati data cast packetu
        std::string data(void);
        //vrati cely packet 
        std::string str(void);

    private:
        long id_num;
        long data_size;
        std::string data_base64;
};

/*
 * @brief trieda pre packet posielany serverom - ACK
 */
class packet_ack
{
    public:
        packet_ack() {};
        packet_ack(int id);
        packet_ack(std::string data);
        //vrati cely packet
        std::string str(void);
        //vrati id cislo packetu
        long id(void);

    private:
        long id_num;
};

#endif /* PACKET_H_ */

