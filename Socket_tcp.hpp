#include "Address.hpp"

//per lavorare con le liste
#include <iostream>
#include <string>
#include <vector>
using namespace std;

#ifndef __SOCKETTCP_HPP
#define SOCKETTCP_HPP

#define MAX_CONNECTIONS 50
#define DEFAULT_SERVER_PORT 8080
#define MAX_BUFFER_SIZE 4096

//TODO Implementare i distruttori


class SocketTcp{

    protected:    int sock_id;

    protected:  SocketTcp();
                ~SocketTcp();

                void setBroadcast(bool);

};

SocketTcp::SocketTcp(){

   /*
        Costruttore della classe principale, imposta il socket:
        - Famiglia: AF_INET
        - Tipo di trasmissione: SOCK_STREAM
        - Flag: 0

        Gestisce mediante errno eventuali errori d'esecuzione

   */

   if (socket(AF_INET, SOCK_STREAM, 0) == -1){
       printf("Error setting socket: %s\n", strerror(errno));
       exit(EXIT_FAILURE);
   }

}

// Distruttore
SocketTcp::~SocketTcp(){}

void SocketTcp::setBroadcast(bool activate_broadcast){

		int isActive = activate_broadcast;

        /*
            Controlla il valore passato alla funzione,
            se l'utente desidera settare il broadcast sul rispettivo
            socket allor si utilizza la funzione setsockopt():

            - Identificativo del socket sul quale impostare il broadcast: this->sock_id
            - Macro per manipolare il socket a livello di API: SOL_SOCKET
            - La caratteristica che si desidera modificare: SO_BROADCAST
            - Il valore per il quale cambiare la suddetta opzione (referenza): &isActive
            - Grandezza del tipo di valore che abbiamo passato: sizeof(bool)

            Naturalmente l'esecuzione della funzione viene, in caso di errori, gestita,
            segnalando all'utente il problema incorso e terminando l'applicazione [se si desidera
            cambiare il valore di ritorno del metodo e non terminare il programma ricordarsi di
            cambiare il tipo restituito dal metodo]

        */

		if (isActive){

			if(setsockopt(
                this->sock_id,
                SOL_SOCKET,
                SO_BROADCAST,
                &isActive,
                sizeof(bool)) == -1 ){

                    printf("Error setting broadcast: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);

                }

		}

	}

//==============================================ServerTcp==================================================================

    /*

        Non è necessario, quando una sottoclasse eredita da una supercalsse, che la prima invochi il costruttore
        della seconda, perchè c++ fa già tutto in automatico, quindi nel caso di ServerTcp (ma anche del client)
        quando verrano invocati verrà prima eseguito il codice all'interno del costruttore di SocketTcp (che apunto
        setta il socket) e successivamente le classi figlie (su cui lavoreranno)

    */

class ServerTcp: private SocketTcp{

    private: int my_port;
             char* my_ip;

             /*
                ad ogni elemento della lista
                passerò un conn_id sul quale i
                metodi lavoreranno

             */
             vector <ServerConnection*> connections;

    /*

        Definisce i 4 costruttori ed il ~distruttore

        ServerTcp(): vuoto, definisce ip e porta di default (vedi macro)
        ServerTcp(bool): se attivo imposta come indirizzo quello default di loopback, altrimenti
        quello DHCP
        ServerTcp(int): offre la possibilita di scegliere la porta, l'indirizzo è settato di default
        ServerTcp(int, bool): imposta sia la porta ricevuta come parametro, che se attivo, l'indirizzo di
        loopback.


    */

    public: ServerTcp();
            ServerTcp(bool);
            ServerTcp(int);
            ServerTcp(int, bool);
            ~ServerTcp();

    private: void bind_cl(Address);
             void listen_cl();
             void accept_connection(Address*);
             void recive_string(char* buffer);
             void send_string(char* buffer);
             Address getMyAddress();

};

ServerTcp::ServerTcp(){

    //setta le variabili di default
    this->my_port = DEFAULT_SERVER_PORT;
    this->my_ip = strdup(IP_LO);

    //riempie un address con l'indirizzo del server, da passare alla bind_cl()
    Address my_self = getMyAddress();
    bind_cl(my_self);

    //mettiamo il server in ascolto per un numero massimo di connessioni
    listen_cl();

}

ServerTcp::ServerTcp(bool is_loopback){

	if (is_loopback)
		this->my_ip = strdup(IP_LO);
	else
		this->my_ip = strdup(IP_DHCP);

	this->my_port = DEFAULT_SERVER_PORT;

    //riempie un address con l'indirizzo del server, da passare alla bind_cl()
    Address my_self = getMyAddress();
    bind_cl(my_self);

    //mettiamo il server in ascolto per un numero massimo di connessioni
    listen_cl();

}

//porta passata dall'utente, indirizzo di default: IP_LO
ServerTcp::ServerTcp(int port){


	this->my_port = port;
	this->my_ip = strdup(IP_LO);

    Address my_self = getMyAddress();
    bind_cl(my_self);

    //mettiamo il server in ascolto per un numero massimo di connessioni
    listen_cl();

	}

	//l'utilizzatore decide a sua discrezione cosa utilizzare
ServerTcp::ServerTcp(int port, bool loopback){

	if (loopback)
		this->my_ip = strdup(IP_LO);
	else
		this->my_ip = strdup(IP_DHCP);

	this->my_port = port;

	Address my_address = getMyAddress();
    bind_cl(my_address);

    //mettiamo il server in ascolto per un numero massimo di connessioni
    listen_cl();

	}

  ServerTcp::~ServerTcp(){

    free(my_ip);

  }

    /*

        Questo metodo ha come unico scopo quello di isolare la chiamata
        alla API di sistema bind_cl(), che associa un struttura sockaddr
        (contenente indirizzo ip e porta), ad un socket.
        Tutti i metodi (costruttori) che necessitano di eseguire questa
        funzione richiamano la seguente, passandole un Address contenente
        appunto porta e ip del server.

        Viene inoltre implementato il controllo su possibili errori in esecuzione,
        e se si verificano l'utente viene avvisato, successivamente l'applicazione
        termina.

    */

//TODO commentare la bind_cl
void ServerTcp::bind_cl(Address my_address){

    sockaddr_in my_self_struct = my_address.getBinary();

    if (bind(sock_id, (struct sockaddr*) &my_self_struct, sizeof(struct sockaddr)) == -1){
        printf("Error bind address and socket: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

}

//TODO commentare la listen_cl
void ServerTcp::listen_cl(){

    if(listen(sock_id , MAX_CONNECTIONS) == -1){
        printf("Error listen_cling for connections: %s\n", strerror(errno));
        exit(EXIT_FAILURE);

    }

}

//TODO commentare la getMyAddress()
Address ServerTcp::getMyAddress(){

    Address my_self;
    my_self.setPort(this->my_port);
    my_self.setIp(strdup(this->my_ip));

    return my_self;

}

void ServerTcp::accept_connection(Address* client_address){

    //mi creo due variabili temporanee con i parametri da passare alla accept()
    struct sockaddr_in my_address = getMyAddress();
    int my_address_size = sizeof(sockaddr);



    //rapido controllo sul valore restituito

    if ((int conn_id = accept(sock_id, (sockaddr*) &my_address, &my_address_size)) == -1){
      printf("Error accepting connection: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }

    /*
      creo un puntatore ad un oggetto 'Connection server'
      ed al costruttore passo il conn_id restituito dalla accept

    */

    ServerConnection* server_connection(conn_id);

    //aggiungo questa Connection alla lista
    connections.push_front(server_connection);

}

void ServerTcp::recive_string(char* buffer){



}


//==============================================ClientTcp===================================================================

class ClientTcp:private SocketTcp{

    private: int dest_port;
             char* dest_ip;

    protected:  ClientTcp();
                void connetti();

};

//costruttore vuoto, commentare
ClientTcp::ClientTcp(){}

void ClientTcp::connetti(){

}


//======================================ServerConnection===================================================================

class ServerConnection: private Connection, private ServerTcp{

    private:  int con_id;
              Connection* my_connection;

    protected:  ServerConnection(int);
                ~ServerConnection();



                /*
                  restituisce NULL in caso di errore
                */
                char* recive_string();

                /*
                  restituisce TRUE in caso di errore
                */
                bool send_string(char);


};

ServerConnection::ServerConnection(int conn_id){

    this.conn_id = conn_id;
    my_connection = new Connection(this.conn_id);

}

ServerConnection::~ServerConnection(){

  free(my_connection);

}

char* ServerConnection::recive_string(){

    //creo un buffer in cui salverò la stringa in arrivo dal client
    char* buffer[MAX_BUFFER_SIZE];

    //ricevo una stringa mediante la ricevi_raw
    //controllo errore facoltativo essendo gia presente nella chiamata alla API
    if(my_connection->recive_raw(buffer, MAX_BUFFER_SIZE))
      printf("+ Attenzione + : errore nel ricevere il messaggio,
              controllare il buffer\n");

    //restituisco la stringa arrivatami dal client
    return buffer;

}

/*
  Invio la stringa al client mediante la manglia my_connection

  In caso di errore: 'return true'
  (l'utente viene comunque avvisato mediante messaggio
  su console che descrive la natura dell'errore)

  Se tutto va bene: 'return false'

*/
bool ServerConnection::send_string(char* string){

    if(my_connection->send_raw(string, strlen(string))
      return true;

    return false;

}



//======================================ConnectionClient===================================================================

class ConnectionClient: private Connection{

  private:  int sock_id;
            Connection* my_connection;

  protected:  ClientConnection(int);
              ~ClientConnection();



              /*
                restituisce NULL in caso di errore
              */
              char* recive_string();

              /*
                restituisce TRUE in caso di errore
              */
              bool send_string(char);


};

ClientConnection::ClientConnection(int socket_id){

  this.sock_id = sock_id;
  my_connection = new Connection(this.sock_id);

}


ClientConnection::~ClientConnection(){

  free(my_connection);

}


char* ClientConnection::recive_string(){

  //creo un buffer in cui salverò la stringa in arrivo dal client
  char* buffer[MAX_BUFFER_SIZE];

  //ricevo una stringa mediante la ricevi_raw
  //controllo errore facoltativo essendo gia presente nella chiamata alla API
  if(my_connection->recive_raw(buffer, MAX_BUFFER_SIZE))
    printf("+ Attenzione + : errore nel ricevere il messaggio,
            controllare il buffer\n");

  //restituisco la stringa arrivatami dal client
  return buffer;

}

/*
Invio la stringa al client mediante la manglia my_connection

In caso di errore: 'return true'
(l'utente viene comunque avvisato mediante messaggio
su console che descrive la natura dell'errore)

Se tutto va bene: 'return false'

*/
bool ClientConnection::send_string(char* string){

  if(my_connection->send_raw(string, strlen(string))
    return true;

  return false;

}


}

//============================================Connection===================================================================

class Connection{

    protected:  Connection(int);

    private:  int id;

              /*
                send (invocata mediante binding dinamico):
                void* -> buffer con risorsa da passare
                int -> grandezza del buffer
                int -> conn_id specificante a quale client mandare il messaggio

                restituisce true in caso di errore

              */
              bool send_raw(void*, int);

              /*
                recive_raw (invocata mediante binding dinamico):
                void* -> buffer con risorsa che arriverà dal client
                int -> grandezza del buffer
                int -> conn_id specificante da quale client ricevere il messaggio

                restituisce true in caso di errore

              */
              bool recive_raw(void*, int);


};

Connection::Connection(int id){

  this.id = id;

}

bool Connection::send_raw(void* buffer, int buffer_size){

    if(send(id, buffer, buffer_size, 0) == -1){
      printf("Error sending buffer: %s\n", strerror(errno));
      return true;
    }

    return false;

}

bool Connection::recive_raw(void* buffer, int buffer_size){

  if(recv(id, buffer, buffer_size, 0) == -1){
    printf("Error receving buffer: %s\n", strerror(errno));
    return true;
  }

  return false;

}


#endif
