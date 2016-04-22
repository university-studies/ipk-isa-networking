/**
 * File: mypmtud.cc
 * Author: Pavol Loffay, xloffa00@stud.fit.vutbr.cz
 * Date: 25.9.2012
 * Description: modul spracujuci parametre prikazoveho riadka
 */


#ifndef PARAMS_H
#define PARAMS_H

#include <string>

/*
 * @brief Trieda pre parametre prikazoveho riadka 
 */
class params
{
    public:
        /*
         * typ parametru z prikazoveho riadka
         */
        enum param_type_enum
        {
            MISSING,
            DOMAIN,
            IPV4,
            IPV6
        };

        params(void);
        void parse(int argc, char * argv[]) throw( const char *);
        long mtu(void) const;
        std::string ip_domain(void) const;
        param_type_enum param_type(void) const;

    private:
        long max_mtu;
        std::string ip_or_domain;
        param_type_enum param;

        enum enum_a
        {
            /*
             * Program ma minimalne 2 a maximalne 4 parametre
             */
            UNDEFINED = -1,
            PARAM_MIN = 2,
            PARAM_MAX = 4
        };
};

#endif // PARAMS_H

