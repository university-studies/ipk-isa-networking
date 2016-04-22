/*
 * Author: Pavol Loffay, xloffa00@stud.fit.vutbr
 * Date: 25.02.2012
 * Project: projekt cislo 1. do predmetu IPK - webovy klient
 * Compiled: gcc 4.5.2
 *
 * evaluation 100%
 */

#include <iostream>
//getopt()
#include <unistd.h>
//string
#include <string>
//memcpy
#include <cstring>
//EXIT_SUCCESS, EXIT_FAILURE 
#include <cstdlib>
//vynimka pre new
#include <exception>
#include <climits>
#include <cerrno>
//TODO co ak je zadane http://

//kniznice pre samotnu komunikaciu
//socket(), connect(), AF_INET, send()
#include <sys/socket.h>
#include <sys/types.h>
//IPPROTO_TCP - definuje TCP protokol
#include <netinet/in.h>
//gethostbyname() - preklad domeny na IPv4
#include <netdb.h>

//vynimka v pripade chyby
const int EXCEPTION = 1;
const int DEFAULT_PORT = 80;
const int DECIMAL_BASE = 10;
const int HTTP_HEAD_SIZE = 10000;
const int MAX_REDIRECT = 5;
const int HTTP_CODE_FOUND = 302;
const int HTTP_CODE_MOVED_PER = 301;
const int HTTP_CODE_OK = 200;
const int PORT_ERROR = -1234;

//struktura pre parametre
struct params_structure
{
    std::string param_buffer;   // ulozia sa tam znaky parametrov po poradi
    std::string url;            // url
    bool help;
};

//struktura popisujuca url
//scheme://domain:port/path?query_string#fragment_id
struct url_structure
{
    std::string scheme;     // http:// 
    std::string domain;     // www.cplusplus.com
    long port;              // port - vacsinou 80
    std::string path;       // /forum/general/7441/ 
};

//struktura popisujuca rozparsovane data ziskane metodou HEAD
struct http_head_structure
{
    std::string all_head;               //cela hlavicka
    std::string object_size;            //-l
    std::string server_identification;  //-s
    std::string last_modifi;            //-m
    std::string object_type;            //-t
    std::string code_message;           //cislo codu a za nim nazov kodu
    int code;                           //200 ak je ok inak chyby
};

const char *help_message = 
    "\tWebinfo: print HTTP HEAD information\n"
    "\tSynopsis: webinfo [-l] [-s] [-m] [-t] URL\n"
    "\t-l: print object size\n"
    "\t-s: print server information\n"
    "\t-m: last object modification\n"
    "\t-t: object type\n"
    "\t-h: help message\n"
    "\tWithout parameters print entire received information\n";

/*
 * funkcia vypise na STDERR chybu a ukonci 
 * @param chybova sprava
 * @param navratovy kod programu
 * @return void
 */ 
void print_error_exit(const std::string & message)
{
    std::cerr << message << std::endl;
    throw EXCEPTION;
}

/*
 * funkcia prevedie string na int a kontroluje chyby
 * @param string kde je cislo
 * @param referencia kde ulozi cislo
 * @return true ak je vsetko OK, inak false;
 */
bool string_to_long(std::string str, long &number)
{
    char * endptr;

    number = strtol(str.c_str(), & endptr, DECIMAL_BASE);
    
    if ((errno == ERANGE && (number == LONG_MAX || number == LONG_MIN)) ||
         (errno != 0 && number == 0))
        return false;

    //no digits were found
    if (endptr == str.c_str())
        return false;

    //zacislo boli este nejake pismena
    if (*endptr != '\0')
        return false;

    return true;
}

/*
 * funkcia na spracovanie IP
 * do scheme ulozi "" ak nebol zadany http://
 * @param string url
 * @return url_structure
 */ 
url_structure url_parse(std::string  url)
{
    //inicializacia
    url_structure url_parsed = {"", "", 0, ""};
    size_t index;

    //PORT must be http://
    index = url.rfind("http://");
    if (! (index == std::string::npos || index != 0))
        url_parsed.scheme = "http://";
    
    std::string pom ("http://");
    url = url.erase(0, pom.length());

    //DOMAIN
    //nenasiel lomitko / - mozebyt alebo nemusi byt zadane
    //priradim celu adresu
    if ((index = url.find('/')) == std::string::npos)
        url_parsed.domain = url;
    
    //priradim domenu po lomitko
    else
        url_parsed.domain = url.substr(0,index);
    
    url = url.erase(0,index);
    
    //PATH
    url_parsed.path = url;

    //PORT
    std::string port;
    index = url_parsed.domain.find(':');
    if (index != std::string::npos)
    {
        //domena obsahuje port
        port = url_parsed.domain.substr(index);
        port = port.erase(0,1);

        if (string_to_long(port, url_parsed.port) == false)
            url_parsed.port = PORT_ERROR;
            
        // z .domain vymazem :XX
        url_parsed.domain = url_parsed.domain.erase(index, 
                url_parsed.domain.length() - 1);
    }
    //domena neobsahje port
    else 
        url_parsed.port = DEFAULT_PORT;

    if (url_parsed.path.empty() == true)
    {
        url_parsed.path = "/";
    }

    return url_parsed;
}

/*
 * funkcia skontroluje URL
 * vypise chybu ak rozparsovana url nie je v poriadku
 * alebo ak nebol zadany protocol http://
 * @param rozparsovana url
 * @return true ak je url v poriadku, inak false
 */
bool check_url(url_structure url)
{
    if (url.scheme == "")
    {
        std::cerr << "Error: wrong URL or protocol!" << std::endl;
        return false;
    }
    if (url.domain == "")
    {
        std::cerr << "Error: wrong URL!" << std::endl;
        return false;
    }
    if (url.port == PORT_ERROR)
    {
        std::cerr << "Error: wrong port!" << std::endl;
        return false;
    }

    return true;
}

/*
 * funkcia spracuje parametre programu
 * @param argc
 * @param argv
 * @return spracovane parametre v strukture params_structure
 */
params_structure get_params(int argc, char * argv[])
{
    params_structure params = {"", "", false};

    int option = 0;
    //getopt() - stderr ak == 0 je chyba
    //optind ukazuje na hodnoty ktore su ulozene za parametrami
    bool increment = false;
    //pretoze optind je pri jednom parametri 2
    int number_of_options = 2; 
    //nastavy aby getopt nevypisoval chyby
    opterr = 0;
    bool bol_cyklus = false;
    while ((option = getopt(argc, argv, "lsmth")) != -1)
    {
        bol_cyklus = true;

        if (increment == true)
            number_of_options++;
        increment = false;

        switch(option)
        {
            case 'l':
                //parameter sa nesmie 2x opakovat
                if (params.param_buffer.find('l') == std::string::npos)
                {
                    // -l este neobsahuje
                    params.param_buffer += 'l';
                }
                increment = true;
                break;
            case 's':
                if (params.param_buffer.find('s') == std::string::npos)
                    params.param_buffer += 's';

                increment = true;
                break;
            case 'm':
                if (params.param_buffer.find('m') == std::string::npos)
                    params.param_buffer += 'm';

                increment = true;
                break;
            case 't':
                if (params.param_buffer.find('t') == std::string::npos)
                    params.param_buffer += 't';

                increment = true;
                break;
            case 'h':
                increment = true;
                params.help = true;
                break;
            default: // '?' 
                print_error_exit("Error params: invalid option!");
                break;
        }

        if (optind >= argc)
        {
            if (params.help == true)
                return params;
            print_error_exit("Error params: URL must be ultimate!");
        }
        if (strlen(argv[optind]) == 1 && number_of_options != optind && params.help == false)
            print_error_exit("Error params: URL must be ultimate!");
    }

    //ake nebol v cykle tak nastavy na number_of_options na 1
    //pretoze optind bude tiez 1
    number_of_options = (bol_cyklus == true) ? number_of_options : 1 ;

    //if (number_of_options != optind && params.help == false)
      //  print_error_exit("Error params: URL must be ultimate!");

    if (params.help == true)
    {
        //param.buffer ma data alebo
        //je nejaka url
        if (params.param_buffer.empty() == false ||
            argc - optind >= 1)
            print_error_exit("Error params: try only -h if you want help!");

        return params;
    }

    //kontrola ci je zadana URL alebo ci je zadanych viac URL
    if (argc - optind > 1)
        print_error_exit("Error params: too much URLs! try -h");
    else if (argc - optind == 0)
        print_error_exit("Error params: missing URL! try -h");

    params.url = argv[optind];

    return  params;
}

/*
 * vytvorit socket(), ak vrati < 0 tak je chyba 
 * @param ur_structure, rozparsovana struktura URL
 * @return data z metody HEAD
 */
std::string get_html_head(url_structure url)
{
    //moje klientske cislo socketu
    int my_socket;
    //informacie o server-e z gethostbyname
    hostent * server_info;

    /*
     * TRANSLATE DOMAIN TO IPv4
     * struct hosten * gethostbyname(const char * name)
     */
    if ( (server_info = gethostbyname(url.domain.c_str())) == NULL)
    {
        //nepodaril sa preklad domeny na IPv4
        print_error_exit("Error: Couldn't translate DOMAIN name to IPv4!");
    }

    /*
     * CREATE MY SOCKET
     * AF_INET - internet domain socket,
     * SOC_STREAM - byte stream socket
     * IPPROTO_TCP - TCP protocol
     * socket (int domain, itn type, int protocol), error if return value < 0
     */
    if ((my_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        // socket sa nepodarilo vytvorit
        print_error_exit("Error: Couldn't create my socket!");
    }

    /*
     * CONNETING TO SOCKET
     * connect(int socket, const struct sockaddr *address, sockle_t address_len)
     * return -1 if error occurs
     * AF_INET - internet domain socket
     */ 
    //adresna struktura pre IPv4
    sockaddr_in sockaddr_item;
    memcpy(&(sockaddr_item.sin_addr), server_info->h_addr, server_info->h_length);
    sockaddr_item.sin_family = AF_INET;
    sockaddr_item.sin_port = htons(url.port);
    if (connect(my_socket, (sockaddr *)&sockaddr_item, sizeof(sockaddr_item)) == -1)
    {
        close(my_socket);
        print_error_exit("Error: Couldn't connect to server!");
    }

    /*
     * Poslanie poziadavku do socketu
     */
    std::string http_request ("HEAD " + url.path + " HTTP/1.1\r\n" +
                             "Host: " + url.domain + "\r\n" +
                             "User-agent: webclient\r\n" +
                             "Connection: close\r\n" +
                             "Accept-language:en\r\n\r\n");

    if(send(my_socket, http_request.c_str(), http_request.size(), 0) == -1)
    {
        close(my_socket);
        print_error_exit("Error: Couldn't send HTTP GET request!");
    }

    /*
     * Nacitanie z mojho socketu co poslal server
     */
    char * http_head_buffer = new char[HTTP_HEAD_SIZE];
    ssize_t recv_ret;
    std::string http_head;
    //musi to byt v cykle pretoze moze poslat viac veci
    while ((recv_ret = recv(my_socket, http_head_buffer, HTTP_HEAD_SIZE, 0)) > 0)
    {   
        http_head += http_head_buffer;
        if (recv_ret == -1)
        {
            close(my_socket);
            delete []http_head_buffer;
            print_error_exit("Error: while receiving data from socket!");
        }      
    }

    //zavretie mojho socketu
    close(my_socket);
    delete []http_head_buffer;

    return http_head;
}

/*
 * vypise data ziskane metodou HEAD
 * @param rozparsovane data z HEAD
 * @return void
 */
void print_html_head(http_head_structure head, params_structure params)
{
    // na zaciatku je odriatkovanie
    std::cout << std::endl;

    //ak je vsetko false vypise celu hlavicku
    if (params.param_buffer.empty() == true)
        std::cout << head.all_head;

    int length = params.param_buffer.length();
    for (int i = 0; i < length; i++)
    {
        switch (params.param_buffer[i])
        {
            case 'l':
                if (head.object_size.empty() == false)
                    std::cout << head.object_size << std::endl;
                else
                    std::cout << "Content-Length: N/A" << std::endl;
                break;
            case 's':
                if (head.object_type.empty() == false)
                    std::cout << head.server_identification << std::endl;
                else
                    std::cout << "Server: N/A" << std::endl;
                break;
                case 'm':
                if (head.last_modifi.empty() == false)
                    std::cout << head.last_modifi << std::endl;
                else
                    std::cout << "Last-Modified: N/A" << std::endl;
                break;
            case 't':
                if (head.object_type.empty() == false)
                    std::cout << head.object_type <<std::endl;
                else
                    std::cout << "Content-Type: N/A" << std::endl;
                break;
            default:
                print_error_exit("Error: interna chyba!");
        }
    }
}

/*
 * funkcia vrati riadok z head kde sa nachadza tooken
 * @param text v ktorom hlada tooken
 * @param tooken ktory sa hlada
 * @return najdeny tooken
 */
std::string parse_line(std::string head, const std::string &token)
{
    std::string line;
    size_t position = 0;

    //true ak najde tooken
    if ((position = head.find(token)) != std::string::npos)
    {
        //havicka obsahuje "Last-Modified:
        //odstrani od zaciatku po najdeny tooken
        head.erase(0, position);
        //odstrany vsetko za riadkom kde je tooken
        if ((position = head.find("\r")) == std::string::npos)
        {
            //neobsahuje \r
            position = head.find("\n");
            head.erase(position);
        }
        else
            head.erase(position);

        line = head;
    }

    return line;
}

/*
 * funkcia rozparsuje HTTP hlavicku, ulozia sa do struktury data 
 * ktore potrebujem
 * @param head nerozparsovana hlavicka
 * @return http_head_structure rozparsovana hlavicka
 */
http_head_structure html_head_parse(std::string head, std::string &url)
{
    http_head_structure head_parsed = {head, "", "", "", "", "", 0};
    std::string pom;
    size_t position;
    size_t position_end;

    //rozparsovanie STAVOVEHO KODU
    position = head.find("HTTP/1.1 ");
    if(position == std::string::npos)
        print_error_exit("Error: HEAD neodpovedal s HTTP/1.0");

    if ((position_end = head.find('\n')) == std::string::npos)
        print_error_exit("Error: nenasiel znak new line v hlavicke");

    pom = head;
    pom = pom.substr(position , position_end);
    //zmaze dlzky "HTTP/1.1 "
    std::string pom2 ("HTTP/1.1 ");
    pom.erase(0, pom2.length());
    //stavove kody su 3 ciferne
    pom.erase(3, position_end);
    
    head_parsed.code = strtol(pom.c_str(), NULL, DECIMAL_BASE);
    //ak je status code 301 alebo 302 zistim novu lokaciu
    if (head_parsed.code == HTTP_CODE_FOUND || 
        head_parsed.code == HTTP_CODE_MOVED_PER)
    {
        //musim najst dalsiu adresu => Location: http://...
        pom2 = "Location: ";
        pom = parse_line(head, pom2);
        pom.erase(0, pom2.length());

        url = pom;
    }
    //vyparsovanie prveho riadku s HTTP/1.1
    pom = parse_line(head, "HTTP/1.1 ");
    pom2 = "HTTP/1.1 ";
    pom.erase(0, pom2.length());
    head_parsed.code_message = pom;

    //parameter -m
    head_parsed.last_modifi = parse_line(head, "Last-Modified: ");
    //parameter -l
    head_parsed.object_size = parse_line(head, "Content-Length: ");
    //parameter -t
    head_parsed.object_type = parse_line(head, "Content-Type: ");
    //parameter -s
    head_parsed.server_identification = parse_line(head, "Server: ");

    return head_parsed;
}

/*
 * funkcia main
 */
int main(int argc, char * argv[])
{
    params_structure params;
    url_structure url_parsed;
    http_head_structure head_struct;
    head_struct.code = HTTP_CODE_FOUND; 
    std::string head;
    try 
    {
        //spracovanie parametrov
        //spracovane budu ulozene v params
        params = get_params(argc, argv);
        
        if (params.help == true)
        {
            std::cout << help_message;
            return EXIT_SUCCESS;
        }

        //kontrola hlbky zanorenia 
        int control = 0;
        while (head_struct.code == HTTP_CODE_FOUND || 
               head_struct.code == HTTP_CODE_MOVED_PER)
        {
            head = "";
            //spracovanie URL
            url_parsed = url_parse(params.url);
           
            if (check_url(url_parsed) == false)
            {
                //control je > 0, musi ist o 301 || 302
                if (control > 0)
                    std::cerr << "Error: Bad redirecting URL!" << std::endl;
                    throw EXCEPTION;
            }
            
            //ziskanie data pomocou metody HEAD
            head = get_html_head(url_parsed);

            //rozparsovanie hlavicky
            head_struct = html_head_parse(head, params.url);
            
            if (head_struct.code != HTTP_CODE_FOUND && 
                head_struct.code != HTTP_CODE_MOVED_PER &&
                head_struct.code != HTTP_CODE_OK)
            {
                std::cerr << std::endl << "Chyba: " << head_struct.code_message
                    << std::endl;
                throw EXCEPTION;
            }

            if (control++ > MAX_REDIRECT)
                print_error_exit("Error: more than 5 redirecting 301 or 302 codes!");
        }

        print_html_head(head_struct, params);
    }

    //moja vynimka EXCEPTION
    catch (int)
    {
        return EXIT_FAILURE;
    }
    //ak sa nepodarila alokovat pamet
    catch (std::bad_alloc&)
    {
        std::cerr << "Error: while allocating dynamin memory!" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

