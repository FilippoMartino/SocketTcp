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

                bool connect();
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

void ClientTcp::connetti(){

}

//============================================Connection===================================================================

class Connection: private ClientConnection{

    protected:  Connection(int);
                ~Connection();

    private:  int id;

              /*
                send (invocata mediante binding dinamico):
                void* -> buffer con risorsa da passare
                int -> grandezza del buffer

                restituisce true in caso di errore

              */
              bool send_raw(void*, int);
              /*
                recive_raw (invocata mediante binding dinamico):
                void* -> buffer con risorsa che arriverà dal client
                int -> grandezza del buffer

                return -1 in caso di errore, altrimenti
                resitiuisce il numero di byte ricevuti

              */
              int recive_raw(void*, int);
              /*
                Invio di una semplice stringa
                appoggiandocisi al metodo send_raw

                return true in caso di errore

              */
              bool send_string(char* buffer);
              /*
                Ricezione di una stringa con aggiunta
                del carattere terminatore per sicurezza

                return null al posto di una stirnga
                nel caso di errore

              */
              char* recive_string();


};

/*

  L'oggetto connessione necessita di un id che consenta
  le operazioni di invio e ricezione: non imprta se si
  riceve un conn_id o un socket_id

*/

Connection::Connection(int id){

  this->id = id;

}

Connection::~Connection(){}

bool Connection::send_raw(void* buffer, int buffer_size){

    if(send(id, buffer, buffer_size, 0) == -1){
      printf("Error sending buffer: %s\n", strerror(errno));
      return true;
    }

    return false;

}

int Connection::recive_raw(void* buffer, int buffer_size){

  int bytes_received = recv(id, buffer, buffer_size, 0);

  if(bytes_received == -1){
    printf("Error receving buffer: %s\n", strerror(errno));
    return -1;
  }

  return bytes_received;

}

bool Connection::send_string(char* buffer){

  int this_buffer_size = strlen(buffer);

  if(send_raw(buffer, this_buffer_size))
    return true;
  else
    return false;

}

char* Connection::recive_string(){

  //buffer in cui verrà salvato il messaggio di cui siamo destinatari
  char buffer[MAX_BUFFER_SIZE];
  //mi serve successivamente per capire dove aggiungere un carattere terminatore
  int this_buffer_size;

  //chiamata alla API
  this_buffer_size = recive_raw(buffer, MAX_BUFFER_SIZE);

  //paradossale controllo per non andare in overflow
  if (this_buffer_size > MAX_BUFFER_SIZE){
    printf("Mmmm...bytes recived are more than the buffer size...\n");
    return NULL;
  }

  //aggiunta del carattere terminatore alla fine della stringa
  buffer[this_buffer_size + 1] = '\0';

  //Ritorno il buffer con il carrattere terminatore aggiunto,
  //dopo averlo clonato in un char* per agevolare chi riceverà il dato
  char* foo;
  strcpy(buffer, foo);
  return foo;

}

//======================================ClientConnection===================================================================

class ClientConnection{

private: ;

protected:  ClientConnection(int);
            ~ClientConnection();

            void send_string(char* message);
            char* recive_string();
};

ClientConnection::ClientConnection(int mmt){

  Connection connection = new Connection(5);

}

//======================================ServerConnection===================================================================

  class ServerConnection:private Connection{

};



#endif
