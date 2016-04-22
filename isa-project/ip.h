/**
 * File: ip.h
 * Author: Pavol Loffay, xloffa00@stud.fit.vutbr.cz
 * Date: 25.9.2012
 * Description: modul obsahujuci ip adresy
 */

#ifndef IP_H
#define IP_H

#include <string>

/*
 * @brief Trieda charakterizujuca ipv4
 */
class ipv4
{
    public:
        ipv4();
        bool parse(std::string str);
        std::string ret(void) const;
        void print(void) const;

    private:
        enum
        {
            IPV4_BLOCKS = 4
        };

        bool test(long);
        long ip[IPV4_BLOCKS];

};

/*
 * @brief Trieda charakterizujuca ipv6
 */
class ipv6
{
    public:
        ipv6();
        bool parse (std::string str);
        std::string ret(void) const;
        void print(void) const;

    private:
        bool test(std::string);
        std::string ip;

   enum
   {
       IPV6_BLOCK_LENGTH = 4
   };
};

#endif // IP_H

