/*
 * Author: Pavol Loffay, xloffa00@stud.fit.vutbr
 * Date: 1.03.2012
 * Project: projekt cislo 2. do predmetu IPK - webovy klient
 * Compiled: gcc 4.5.2
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
#include <arpa/inet.h>
#include <signal.h>

#include <signal.h>
#include <sys/wait.h>

using namespace std;

const int EXCEPTION = 1;
const int DECIMAL_BASE = 10;
const int MAX_CONNECTIONS = 100;
const int MAX_ANSWER_SIZE = 10000;
const int MAX_IP_LEN = 100;
const int RETURN_CLIENT = 5;
const int RETURN_SERVER = 4;

//help sprava
const char * help_message = 
    "\tServer: run concurent server\n"
    "\tSynopsis: server -p port_number\n";

/*
 * struktura popisujuca spracovane parametre
 */
struct params_struct
{
    long port;   //cislo portu tohto servera
    bool help;
};

//struktura popisujuca klienta
struct client_info
{
    sockaddr_in addr_info;
    int number;
};

//struktura popisujuca rozparsovanu poziadavku od klienta
struct request_parsed_struct
{
    bool ipv4;
    bool ipv6;
    string domain;
};

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
bool string_to_long(string str, long &number)
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
 * funkcia spracuje prarametry programu
 * @param argc
 * @param argv
 * @return spracovane parametre v strukture
 */
params_struct get_params(int argc, char * argv[])
{
    params_struct params = {0, false};
    
    bool port = false;
    int option = 0;
    //aby getopt nevypisoval chyby
    opterr = 0;
    while ((option = getopt(argc, argv, "hp:")) != -1)
    {
        switch(option)
        {
            case 'p':
                port = true;
                if (optarg != NULL)
                {
                    if (string_to_long(string(optarg), params.port) == false)
                        print_error_exit("Error: failed to convert port to number!");
                }
                else
                    print_error_exit("Error params: -p require argument!");
                
                break;
            case 'h':
                params.help = true;
                break;

            default: // '?'
                print_error_exit("Error params: invalid option! Try -h!");
                break;
        }
    }

    if (params.help == true && port == true)
        print_error_exit("Error params: try only -h if you want help!");

    if (port == false && params.help == false)
        print_error_exit("Error params: musit run with -p port_number! Try -h!");

    if (argc != 3 && argc != 2)
        print_error_exit("Error params: try -h");
    return params;
}


/*
 * funkcia rozparsuje poziadavu od klienta
 * @param poziadavka
 * @return struktura spracovanej poziadavky
 */
request_parsed_struct request_parse(string request)
{   
    request_parsed_struct parsed = {false, false, ""};

    if (request.find("GET_IPV4 ") != string::npos)
    {
        //budem prekladat do IPv4
        parsed.ipv4 = true;
    }
    else if (request.find("GET_IPV6 ") != string::npos)
    {
        parsed.ipv6 = true;
    }
    else if (request.find("GET_IPIP ") != string::npos)
    {
        parsed.ipv4 = true;
        parsed.ipv6 = true;
    }
    else
        print_error_exit("Error: couldn't parse request from client!");

    //rozparsovanie domeny
    string pom ("GET_IPV4 ");
    
    parsed.domain = request.substr(pom.length(), request.length());

    size_t index = parsed.domain.find(" ");
    if (index != string::npos)
        parsed.domain = parsed.domain.substr(0, index);
    else
        print_error_exit("Error: couldn't parse request from client aa!");

    return parsed;
}

/*
 * funkcia zisti IP adresu z domeny
 * @param nazov domeny
 * @param struktura prijatej poziadavky od klienta
 * @return ziskane IP adresi
 */
string get_ip(request_parsed_struct request)
{
    string ip = "";
    
    //potrebne struktury
    addrinfo hints;
    addrinfo *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_CANONNAME;
            
    if (getaddrinfo (request.domain.c_str(), NULL, &hints, &res) != 0)
    {
        //neziskal ziadnu IP
        ip = "NOT_FOUND ";
        return ip;
    }

    char ip_str[MAX_IP_LEN];
    void * ptr;
    for (; res != NULL; res = res->ai_next)
    {

        if (res->ai_family == AF_INET6)
        {
            inet_ntop (res->ai_family, res->ai_addr->sa_data, ip_str, MAX_IP_LEN);
            ptr = &((struct sockaddr_in6 *) res->ai_addr)->sin6_addr;
        }
        else if(res->ai_family == AF_INET)
            ptr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
        
        inet_ntop (res->ai_family, ptr, ip_str, MAX_IP_LEN);
        
        if (res->ai_family == PF_INET6 && request.ipv6 == true)
            ip += "IPV6 " + string(ip_str) + " ";
        if (res->ai_family != PF_INET6 && request.ipv4 == true)
            ip += "IPV4 " + string(ip_str) + " ";
    }
    
    /*
      pri testovani na eve.fit.vutbr to sposobilo chyby!!! proces tu ostal stat
    */
    //freeaddrinfo(res);

    if (ip.empty() == true)
        ip = "NOT_FOUND ";

    return ip;
}

/*
 * funkcia pre obsluzenie klienta
 * @param socket klientskeho spojenia
 * @return true ak vsetko prebehlo OK, inak false
 */
bool client_service(int client_socket)
{
    /*
     * citanie zo socketu co poslal server
     */
    string answer;
    char * answer_buffer = new char[MAX_ANSWER_SIZE];
    ssize_t recv_ret;
    recv_ret = recv(client_socket, answer_buffer, MAX_ANSWER_SIZE, 0);
    if (recv_ret == -1)
    {
        close(client_socket);
        delete []answer_buffer;
        cerr << "Error: while receiving data from socket!";
        return false;
    }
    string request = answer_buffer;
    delete []answer_buffer;
    

    request_parsed_struct parsed = request_parse(request);
    string to_send = get_ip(parsed);


    /*
     * send spracovane data klientovy
     */
    send(client_socket , to_send.c_str(), to_send.length(), 0);
                     
    return true;
}

/*
 * funckia sposoby ze proces servera bude pokracovat a pritom
 * prevezme navratovy kod klienta
 * @param int
 * @return void
 */
void my_reaper(int )
{
    int status;
    while (wait3(&status, WNOHANG, NULL) >= 0);

}

/*
 * hlavna funkcia servera
 * vytvory port kde bude pocuvat, pre kazdeho kienta novy proces
 * @param port na ktorom pocuva
 * @return CLIENT_RETURN ak skoci proces kienta, 
 *         SERVER_RETURN ak skoci processerver
 */
int server(int port)
{
    int my_socket;

    /*
     * Vytvorenie socketu
     */
    if ((my_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        print_error_exit("Error: Couldn't create my socket!");

    sockaddr_in socket_addr_in;
    socket_addr_in.sin_family = AF_INET;
    socket_addr_in.sin_port = htons(port);
    //TODO nechat iba prvu strukturu
    socket_addr_in.sin_addr.s_addr = INADDR_ANY;

    /*
     * Bind
     */
    if (bind(my_socket, (sockaddr *) &socket_addr_in, sizeof(socket_addr_in)))
    {
       close(my_socket);
       print_error_exit("Error: bind() failed!");
    }
    
    /*
     * Listen
     */
    if (listen(my_socket, MAX_CONNECTIONS) == -1)
    {
        close(my_socket);
        print_error_exit("Error: listen() failed!");
    }

    int client_socket;
    pid_t client_pid;
    while(true)
    {
        client_info client_struct;
        /*
         * Accept
         * z fronty zobere prveho cakatela na spojenie 
         * a vytvory novy socket s takym instym typom spojenia pre klienta
         */
        socklen_t len = sizeof(client_struct.addr_info);


        if ((client_socket = accept(my_socket, (sockaddr *)
                                   &(client_struct.addr_info),&len)) == -1)
        {
            close(my_socket); //TODO zavriet klientske spojenia
            print_error_exit("Error: accept() failed!");
        }
    
        client_pid = fork();
        if (client_pid == 0)
        {
            //kod potomka   
            close(my_socket);         
            client_service(client_socket);            
            close(client_socket);
            return RETURN_CLIENT;
        }
        else if (client_pid == -1)
        {
            /*
             * Nepodareny fork, TODO odstranit vsetko
             */

            close(my_socket);
            close(client_socket);
            //TODO zavret sockety klientov
            kill(getuid(), SIGKILL);
            print_error_exit("Error: fork() failed!");
        }

        //kod hlavneho procesu - procesu servera
        //musi pockat na odstavenie vsetkych klientov
        close(client_socket);

        signal(SIGCHLD, &my_reaper);
        //waitpid(client_pid, NULL, 0);
    }
    
    close(my_socket);
    return RETURN_SERVER;
}

/*
 * Funkcia sa voal po odchiteni signalu
 * @param signal cislo signalu
*/
void signal_catcher(int signal)
{
    signal = 0;
    exit(0);
}

/*
 * funkcia main
 */
int main(int argc, char * argv[])
{
    signal(SIGINT, signal_catcher);
    signal(SIGTERM, signal_catcher);

    try
    {
        params_struct params = get_params(argc, argv);
        if (params.help == true)
        {
            cout << help_message;
            return EXIT_SUCCESS;
        }

        int ret;
        ret = server(params.port);

    }
    catch(int)
    {
        return EXIT_FAILURE;
    }
    catch (bad_alloc&)
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


