#include "address.hpp"

#include <unistd.h>
#include <list>
#include <iterator>

using namespace std;

#ifndef __SOCKETTCP_HPP
#define SOCKETTCP_HPP

#define MAX_CONNECTIONS 50
#define DEFAULT_SERVER_PORT 8080
#define MAX_BUFFER_SIZE 4096

//TODO Implementare i distruttori


class SocketTcp{

    protected:  int sock_id;

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

   if (this->sock_id= socket(AF_INET, SOCK_STREAM, 0) == -1){
       printf("Error setting socket: %s\n", strerror(errno));
       exit(EXIT_FAILURE);
   }

}

// Distruttore
SocketTcp::~SocketTcp(){

  //mi limito a chiudere il socket
  close(this->sock_id);

}

void SocketTcp::setBroadcast(bool is_active){


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

		if (is_active){

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

class ServerTcp: private SocketTcp{

    private:  list <ServerConnection*> connections;
              char* my_ip;

    public:   ServerTcp(int);
              ~ServerTcp();
              ServerConnection* accept();
              //Chiude una singola connessione
              void disconnect (ServerConnection*);
              //Chiude tutte le connessioni presenti nel server
              void server_shutdown();
              //TODO da implementare le due funzioni sottostanti
              //bool send_multicast_message(char*);
              //bool send_multicast_raw(void*, int);

    private:  void close_connections();

};

//associo il socket all'indirizzo
ServerTcp::ServerTcp(int port){

    //Mi costruisco un indirizzo con la classe Address
    Address my_address(strdup(IP_LO), port);
    //Inserisco il risultato nella struttura binaria
    struct sockaddr_in my_self = my_address.getBinary();

    //associo indirizzo a socket
    if (bind(sock_id,
            (struct sockaddr*) &my_self,
            (socklen_t) sizeof(struct sockaddr_in))
            == -1)
    //Intercetto eventuale presenza errore
    printf("Error doing bind(): %s\n", strerror(errno));

    //Metto il server in ascolto
    if (listen(sock_id, MAX_CONN) == -1)

      //Intercetto eventuale presenza errore
      printf("Error listening for client: %s\n", strerror(errno));

}

//Chiude tutte le connessioni legate a questo socket
ServerTcp::~ServerTcp(){

  disconnect();

}

ServerConnection* ServerTcp::accept(){

  //Struttura in cui finisce l'indirizzo del client che ci sta contattando
  struct sockaddr client_address;
  int struct_len = sizeof(struct sockaddr);

  //chiamo la API ed intercetto eventuali errori
  int conn_id = accept(sock_id,
                      (struct sockaddr*) &client_address,
                      (socklen_t*) &struct_len);

  if (conn_id == -1)
    printf("Error accepting connection: %s\n", strerror(errno));

  //Creo la nuova connessione con il conn_id accettato
  ServerConnection* ret = new ServerConnection(conn_id);
  //Aggiungo la connessione alla lista
  connections.push_front(ret);

  //Restituisco la nuova connessione
  return ret;

}

void ServerTcp::disconnect(ServerConnection* connection){

  //Rimuove la connessione dalla lista
  connections.remove(connection);
  //Elimina l'oggetto
  delete(connection);

}

void ServerTcp::server_shutdown(){

  //Cicla finchè la lista delle connessioni non è vuota
  while (!connections.empty()){
    //disconnette la connessione all'inizio della lista
    disconnect(connections.front());
  }

}


//==============================================ClientTcp===================================================================

class ClientTcp:private SocketTcp{

    //Puntatore ad una connessione client, inizializzato a null
    private: ClientConnection* connection = NULL;

    protected:  ClientTcp();
                ~ClientTcp();

                /*
                * Questi metodi sono in realtà fittizzi, per ognuno
                * ci si appoggia alla classe che gestisce le connessioni
                * del client, a cui si accede mediante la maniglia 'connection',
                * che viene inizializzata quando di invoca il metodo connect
                *
                */

                bool connect(Address);
                bool send_message(char*);
                bool send_raw(void*, int);
                char* receive_message();
                char* receive_raw(int*);
};

ClientTcp::ClientTcp():SocketTcp{}

ClientTcp::~ClientTcp(){

  if(connection != NULL)
      delete connection;

}

bool ClientTcp::connect(Address server_address){

  struct sockaddr_in server = serverAddress.getBinary();

  //Connetto il socket al server, intercetto eventuali errori
  if (connect(sock_id,
             (struct sockaddr*) &serverAddr,
             sizeof(struct sockaddr_in)) == -1){

               printf("Error connecting to server: %s\n", strerror(errno));
               return true;

             }

  this->connection = new ClientConnection(sock_id);
  return false;

}

//============================================Connection===================================================================

class Connection{

    protected:  int conn_id;

    public: Connection(int);
  		      ~Connection();

            bool send_message(char*);
  		      bool send_raw(void*,int);
  		      char* receive();
  		      char* receive_raw(int*);

};

Connection::Connection(int id){

  this->conn_id = id;

}

Connection::~Connection(){}

bool Connection::send_raw(void* buffer, int buffer_size){

    if(send(this->conn_id,
            buffer,
            buffer_size,
            0) == -1){

      printf("Error sending buffer: %s\n", strerror(errno));
      return true;

    }

    return false;

}

bool Connection::send_message(char* message){

  return send_raw(message, strlen(message) + 1);

}

void* Connection::recive_raw(int* buffer_size){

  char buffer[MAX_MESSAGE + 1];

  //Nel valore puntato da buffer_size inserisco il numero di bit ricevuti
  buffer_size = recv(this->connID,
                      buffer,
                      MAX_MESSAGE,
                      0);
	if(buffer_size == -1)
		printf("Error receiving buffer: %s", strerror(errno));

	buffer[MAX_MESSAGE + 1] = '\n';
  char* foo;
  strcpy(buffer, foo);
  return (void*) foo;

}

char* Connection::recive_message(){

  int message_lenght;
  char* message = (char*) receive_raw(&message_lenght);
  //Ulteriore controllo sul carattere terminatore
  //Sposto il puntatore sull'ultima cella e vado ad aggiungerlo
  *(message + lenght) = '\0';

  //restistuisco il messaggio
  return message;

}

//======================================ClientConnection===================================================================

class ClientConnection: protected Connection{

public: ClientConnection(int);
        ~ClientConnection();

};

ClientConnection::ClientConnection(int conn_id):
  Connection(conn_id){}

ClientConnection::~ClientConnection(){}

//======================================ServerConnection===================================================================

class ServerConnection:protected Connection{

  public: ConnServer(int);
		      ~ConnServer();
};

ConnServer::ConnServer(int conn_id):Connection(conn_id){}

ConnServer::~ConnServer(){

	shutdown(connID, SHUT_RDWR);

}



#endif
