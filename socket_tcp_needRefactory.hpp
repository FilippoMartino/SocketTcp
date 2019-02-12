  #ifdef __SOCKET_TCP_HPP__
  #define	__SOCKET_TCP_HPP__


  #include <stdio.h>
  #include "Address.hpp"
  #include <stdlib.h>
  #include <string.h>
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  #include <error.h>

  #define MAX_MESSAGE 4096
  #define MAX_CONN 50
  #define  IP_LO "127.0.0.1"
  #define IP_DHCP "0.0.0.0"


  /*=============================SoketTcp=============================*/


  class SocketTcp{

	  protected:  int socket_id;

	  public:		SocketTcp();
		          void setBroadcast(bool);
		          ~SocketTcp();	
  };

  SocketTcp::SocketTcp(){

		  this->socket_id = socket(AF_INET, SOCK_STREAM, 0);		
  }

  SocketTcp::setBroadcast(bool broadcast_status){
	  
      int broadcast = broadcast_status;
	  if(setsockopt(this->socket_id, SOL_SOCKET, SO_BROADCAST, (void*) broadcast, sizeof(bool) ) < 0){
		  printf("Error setting broadcast: %s\n", strerror(errno));
          return -1;	

      }
  }

  SocketTcp::~SocketTCP(){}

  /*=============================Connection=============================*/

  class Connection{
   
	  private:    Address* host;
		          int conn_id;

	  public: 	Connection(int);
		          ~Connection();
          		int send(char*);
		          char* receive();
		          void setHost(Address*);
		          Address* getHost();
		          bool sendRaw(void*,int);
		          void* receiveRow(int*);	

  };

  Connection::Connection(int conn_id){
	  if(conn_id > 0) this->conn_id = conn_id;
	  
  }


  Connection::~Connection(){
      free(host);
  } 	

  bool Connection::send(char* message){
    
    //la send raw ha comunque un valore di ritorno
	  return sendRaw(message, strlen(message)+1);	
	  
  }

  char* Connection::receive(){

	  int lenght;	
    
    //Riceve il messaggio e gli passa il carattere di controllo
	  char* ret = (char*) receiveRaw(&lenght);
	  *(ret + lenght) = '\0';

	  return ret
	  
  }


  bool Connection::sendRaw(void* message, int* msg_lenght){

	  int ret = send(conn_id, message, msg_lenght, 0);
	  return (ret != len);
	  
  }

  void* Connection:receiveRaw(int* msg_lenght){

	  char buffer[MAX_MESSAGE+ 1];

	  *msg_lenght = recv(this->conn_id,(void*) buffer, MAX_MESSAGE, 0);

	  void* ret = malloc(*msg_lenght);	
	  
	   //rifattorizzare questo punto
	  
	  for(int i=0; i<=*len; i++){
		  
		  *(ret+1) =buffer[i];
	  
	  }
		  
	  return (void*) ret;
  
  }
 
  /*=============================ClientConnection=============================*/
 
  ClientConnection:public Connection{
	  
	  public: ClientConnection(int);
	          ~ClientConnection();	
  
  };

  //richiama il costruttore della superclasse generalizzata connessione
  //++ Non sono sicuro che sia necessatio invocare il costruttore ++
  ClientConnection::ClientConnection(int conn_id): Connection(conn_id){}

  ClientConnection::~ClientConnection(){}

  /*=============================ClientTcp=============================*/


  class ClientTcp: public SocketTcp {
	  private:    ClientConnection* connection = NULL;
		  
	  public:		ClientTcp();
		          ~ClientTcp();

		          bool send(char*);
    		      char* receive();
		          bool sendRaw(void*,int);
        		  void* receiveRaw(int*);
		          bool connetti(Address);	
  }

  ClientTcp::ClientTcp(){} //la chiamata al costruttore della sopraclasse viene effettuata in automatico

  ClientTcp::~ClientTcp(){
	  if(connection != NULL)
		  delete connection;
  }

  bool ClientTcp::send(char* message){
	  return connection -> send(message);
  }

  bool ClientTcp::sendRaw(void* message, int lenght){
	  return connection -> sendRaw(message, lenght);
  }

  char* ClientTcp::receive(){
	  return connection -> receive();
  }

  char* ClientTcp::receiveRaw(int* lenght){
	  return connection -> receiveRaw(lenght);
  }

  bool ClientTcp::connetti(Address server_address){
	  
	  struct sockaddr_in server_addr = server_address.getBinary();

	  int ret = connect(socket_id,(struct sockaddr*)&server_addr,sizeof(struct sockaddr_in));

	  if(ret == 0){
		  connection = new ClientConnection(socket_id);
		  return true	
	  }
	  
	  return false;
  }

  /*=============================ServerConnection=============================*/

  class ServerConnection : public Connessione{

	  public: ServerConnection(int);
        	  ~ServerConnection();
  
  };
  
  //vale lo stesso discorso di prima
  ServerConnection::ServerConnection(int conn_id): Connection(conn_id){}

  ServerConnection::~ServerConnection(){shutdown(conn_id, SHUT_RDWR)}	

  /*=============================ServerTcp=============================*/

  class ServerTcp : public SocketTCP {
	  
	  private: list<ServerConnection*> connections; 
			   
	  public: ServerTcp(int port);
		        ServerConnection* accept();
		        void disconnect(ServerConnection*);
		        bool sendEveryWhere(char*);
      		  bool sendEveryWhereRaw(void*, int);
		        char* receive(int);
		        ~ServerTcp();

	  private: void chiudiConnessioni();
	  
  };

  ServerTcp::ServerTcp(int port){
	  
	  Address address(IP_DHCP, port);	
	  struct sockaddr_in myself = address.getBinary();
		  
	  //todo controllo sulla bind
	  bind(socket_id,(struct sockaddr*) &myself,(socklen_t) sizeof(struct sockaddr_in));

	  listen(socket_id,MAX_CONN );
  }

  ServerTcp::~ServerTcp(){
	  chiudiConnessioni();
	  
  }


  ServerConnection* ServerTcp::accept(){
	  ServerConnection* ret;
	  int conn_id;
	  struct sockaddr client;
	  int len_addr = sizeof(struct sockaddr);

	  conn_id = accept(socket_id,(struct sockaddr*)&client,(socklen_t*)&len_addr);

	  ret = new ServerConnection(conn_id);
	  connections.push_front(ret);

	  return ret;
  }

  void ServerTcp::disconnect(ServerConnection* conn){
	  delete(conn);
	  connections.remove(conn);
  }


  void ServerTcp::chiudiConnessioni(){
	  
	  while(!connections.empty()){	//cicla finchè la lista non è vuota
			  disconnect(connections.front());	//.front restituisce la prima connessione della lista
	  }
  }

  void ServerTcp::sendEveryWhere(char* msg){
	  sendEveryWhereRaw(msg,srtlen(msg+1));
  }

  bool ServerTcp::sendEveryWhereRaw(void* msg, int len){
	  
	  bool ret = true;
	  for(list<ServerConn*>::itarator it = connections.begin();it != connections.end(); it++){	//scorre tutta la lista
		  ret = ret && *it -> sendRaw(msg, len);
	  }

	  return ret;
  }


  /*===========================================================*/

  #endif __SOCKET_TCP_HPP__



   
