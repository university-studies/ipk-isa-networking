/**
 * Author: Pavol Loffay, xloffa00@stud.fit.vutbr.cz
 * Date: 14.04.2012
 * Project: Projekt 3. do predmetu IPK
 *          implementacia spolahliveho prenosu pomocou RDT
 *          pouzity protokol je UDP
 * Module: Modul spracujuci parametre prikazoveho riadka
 */

#ifndef PARAMS_H
#define PARAMS_H

#include <string>


/*
 * @brief trieda pre vynimku ktora sa pouziva v params
 */
class param_exception
{
    public:
        param_exception(std::string err);
        std::string err(void);
    private:
        std::string mesg;
};

/*
 * @brief trieda pre parametre skriptu
 */
class params
{
    public:
        params(void);
        void parse(int argc, char * argv[]) throw(param_exception); 
        long dest(void) const;
        long source(void) const;

    private:
        long source_port;
        long dest_port;
        bool string_to_long(std::string str, long &number);

        enum
        {
            /*
             * program musi mat 4 pramametre
             */
            PARAM_NUMBER = 4,
            DECIMAL_BASE = 10
        };
};

#endif // PARAMS_H

