/*
 * Author: Pavol Loffay, xloffa00@stud.fit.vutbr
 * Date: 1.03.2012
 * Project: projekt cislo 2. do predmetu IPK - webovy klient
 * Compiled: gcc 4.5.2
 *
 * evaluation: 100%
 */

#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <cerrno>
#include <climits>
#include <cstring>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

using namespace std;
const int EXCEPTION = 1;
const int DECIMAL_BASE = 10;
const int OPTIND_THREE = 3;
const int OPTIND_FOUR = 4;
const int MAX_ANSWER_SIZE = 10000;

//struktura popisujuca spracovane parametre programu
struct params_struct
{
    long port;              //cislo portu serveru
    string buffer;          //ulozene parametre -4 -6
    string server;          //identifikacia serveru - nazov
    string domain;          //domenove meno klienta
};

//ziskane data z serveru
struct ip_struct
{
    string ipv4;
    string ipv6;
};

//help sprava
const string help_message = 
    "\tClient: connect to server and translate domain to IPv6 or IPv4\n"
    "\tSynopsis: client SERVER_NAME:PORT [-6] [-4] DOMAIN_NAME\n"
    "\t-6: translate to IPv6\n"
    "\t-4: translate to IPv4\n"
    "\t must rum with -4 or -6";

/*
 * funkcia vypise na STDERR chybu a ukonci 
 * @param chybova sprava
 * @return void
 */ 
void print_error_exit(const std::string & message)
{
    cerr << message << endl;
    throw EXCEPTION;
}

/*
 * funkcia prevedie string na int a kontroluje chyby
 * @param string kde je cislo
 * @param referencia kde ulozi cislo
 * @return true ak je vsetko OK, inak false;
 */
bool string_to_int(string str, long &number)
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
 * funkcia rozparsuje parameter programu server_name:port
 * @param string server_name:port
 * @param referencia kde sa ulozi string server_name
 * @param referencia kde sa ulozi int port
 * @return true ak je vsetko OK, inak false
 */
bool parse_name_port(string name_port, string &name, long &port)
{
    size_t index = 0;
    string port_str = "";

    index = name_port.rfind(':');
    if (index == string::npos)
        return false;
    
    name = name_port.substr(0, index);
    //+1 pretoze : nesmie byt v porte
    port_str = name_port.substr(index + 1, name_port.length());

    if (string_to_int(port_str, port) == false)
        return false;

    return true;
}

/*
 * funkcia na spracovanie parametrov
 * @param argc
 * @param argv
 * @return spracovane parametre v strukture
 */
params_struct get_params(int argc, char * argv[])
{
    params_struct params = {0 ,"", "", ""};

    int option = 0;
    opterr = 0; //turn off getopt error message
    extern int optind;
    optind = 2;
    while ((option = getopt(argc, argv, "46")) != -1)
    {
        switch(option)
        {
            case '4':
                if (params.buffer.find('4') == string::npos)
                    params.buffer += '4';
                break;

            case '6':
                if (params.buffer.find('6') == string::npos)
                    params.buffer += '6';
                break;

            default: //'?'
                print_error_exit(help_message);
                break;
        }

        if (optind != OPTIND_THREE && optind != OPTIND_FOUR)
            print_error_exit(help_message);

        // ./server localhost::port www.a.cz -[46]
        if (optind == OPTIND_FOUR && params.buffer.length() == 1)
            print_error_exit(help_message);
    }

    //neboli zadane 2 povinne parametre SERVER_NAME:PORT a DOMAIN
    if (argc - optind != 1)
        print_error_exit(help_message);

    //nebol zadany jeden parameter -4 alebo -6
    if (params.buffer.empty() == true)
        print_error_exit(help_message);

    //server_name:port
    if (parse_name_port(argv[argc - optind], params.server, params.port) == false)
        print_error_exit("Params error: could not parse port number!");

    //domain_name
    params.domain = argv[optind];
    
    return params;
}

/*
 * funkcia posle poziadavok na server a prime data
 * @param socket
 * @param poziadavok
 * @param odpoved
 */
void get_answer(int socket, string request, string &answer)
{
    if (send(socket, request.c_str(), request.size(), 0) == -1)
    {
        close(socket);
        print_error_exit("Error: Couldn't send request to server!");
     }

    /*
     * citanie zo socketu co poslal server
     */
    char * answer_buffer = new char[MAX_ANSWER_SIZE];

    ssize_t recv_ret;
    recv_ret = recv(socket, answer_buffer, MAX_ANSWER_SIZE, 0);
    if (recv_ret == -1)
    {
        close(socket);
        delete []answer_buffer;
        print_error_exit ("Error: while receiving data from socket!");
    }

    answer += answer_buffer;
    delete []answer_buffer;
}

/*
 * funkcia sa pripoji na server a kde posle domenu 
 * a ta sa prelozi podla parametrov porgramu na ipv4 adresu alebo ipv6
 * @param spracovane parametre programu
 * @return prelozena domena na ip adresy.
 */
string translate_domain(params_struct params)
{
    //informacie o server - ziskanie jeho IP
    hostent * server_info;
    int my_socket;

    /*
     * translate DOMAIN to IPv4
     */
    if ((server_info = gethostbyname(params.server.c_str())) == NULL)
        print_error_exit("Error: Couldn't translate server_name to IPv4!");

    /*
     * Create my SOCKET
     */
    if ((my_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        print_error_exit("Error: Couldn't create my socket!");

    /*
     * Connecting to socket
     */
    sockaddr_in sockaddr_item;
    memcpy(&(sockaddr_item.sin_addr), server_info->h_addr, server_info->h_length);
    sockaddr_item.sin_family = AF_INET;
    sockaddr_item.sin_port = htons(params.port);
    if (connect(my_socket, (sockaddr *)&sockaddr_item, sizeof(sockaddr_item))
            == -1)
    {
        close(my_socket);
        print_error_exit("Error: Couldn't connect to server!");
    }

    string answer;
    /*
     * Poslanie poziadavku na server
     */
    if (params.buffer.find('4') != string::npos && 
        params.buffer.find('6') == string::npos)
    {
        //poslanie poziadavku nech prelozi IPv4 a prijatie adresy
        string request("GET_IPV4 " + params.domain + " \r\n\r\n");
        get_answer(my_socket, request, answer);

    }   
    if (params.buffer.find('6') != string::npos && 
        params.buffer.find('4') == string::npos)
    {
        //poslanie poziadavku nech prelozi IPv6 a prijatie adresy
        string request ("GET_IPV6 " + params.domain + " \r\n\r\n");
        get_answer(my_socket, request, answer);
    }
    else if (params.buffer.find('4') != string::npos && 
             params.buffer.find('6') != string::npos)
    {
        //poslanie poziadavky pre ipv4 aj ipv6
        string request("GET_IPIP " + params.domain + " \r\n\r\n");
        get_answer(my_socket, request, answer);
    }

    close(my_socket);
    return answer;
}

/*
 * funkcia vytlaci podla parametrov skriptu prelozenu
 * domenu na ipv4 alebo ipv6 adresy
 * @param ip adresy
 * @param parametre programu
 * @return void
 */
void print_ip(ip_struct ip, params_struct params)
{
    cout << endl;

    int length = params.buffer.length();
    for(int i = 0; i < length; i++)
    {
        switch(params.buffer[i])
        {
            case '4':
                if (ip.ipv4.empty() == false)
                    cout << ip.ipv4 << endl;
                else
                    cerr << "Err4: Nenalezeno." << endl;
                break;
            case '6':
                if (ip.ipv6.empty() == false)
                    cout << ip.ipv6 << endl;
                else
                    cerr << "Err6: Nenalezeno." << endl;
                break;
             default:
                print_error_exit("Error: intern error!");    
                break;
        }
    }
}

/*
 * funkcia skonvertuje odpoved zo serva
 * @param odpoved zo servera
 * @param ktora akresa sa ma hladat "IPV4 " "IPV 6"
 * @return najdena adresa
 */
string answer_parse(string answer,const string  &needle)
{
    string ret = "";
    size_t index;

    string pom = answer;
    index = answer.find(needle);
    if (index != string::npos)
    {
        //obsahuje IPV4 alebo IPV6 
        //vymazem vsetko do needle
        pom.erase(0, index);
        //vymazem "needle"
        index = pom.find(needle);
        pom.erase(index, needle.length());
        
        index = pom.find(" ");
        if (index != string::npos)
            pom = pom.substr(0, index);
        
        ret = pom;
    }

    return ret;
}

/*
 * funkcia main
 */
int main(int argc, char * argv[])
{
    try
    {
        params_struct params = get_params(argc, argv);

        string answer = translate_domain(params);

        ip_struct ip = {"", ""};
        
        ip.ipv4 = answer_parse(answer, "IPV4 ");
        ip.ipv6 = answer_parse(answer, "IPV6 ");

        print_ip(ip, params);
    }
    catch(int)
    {
        return EXIT_FAILURE;
    }
    catch(bad_alloc&)
    {
        cerr << "Error: while allocating dynamic memory!" << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

