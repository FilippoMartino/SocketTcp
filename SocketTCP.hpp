

#include "address.hpp"
#include <unistd.h>
#include <list>
#include <iterator>
using namespace std;
#ifndef __SOCKETTCP_HPP__
#define __SOCKETTCP_HPP__
#define MAX_MESSAGE 4096
#define MAX_CONN 50


/*==================================================================*/
class SocketTCP{
	protected:	int sock_id;
	public:		SocketTCP();
			void setBroadcast(bool);
			~SocketTCP();

};
SocketTCP::SocketTCP(){
	printf("Apro il socket...");
	this->sock_id=socket(AF_INET,SOCK_STREAM,0);
	if(sock_id == -1)
		printf("Error setting socket id: %s", strerror(errno));
	else
		printf("OK\n");
		
}

void SocketTCP::setBroadcast(bool attivo){
	int optval;
	int ret;
	if (attivo) {
		ret = setsockopt(this->sock_id,SOL_SOCKET,SO_BROADCAST,&optval,sizeof(optval));
		if (ret)
			printf("Error setting broadcast: %s", strerror(errno));
		
	}
}
SocketTCP::~SocketTCP(){
	printf("Chiudo il socket...");
	close(this->sock_id);
	printf("OK\n");
}
/*==================================================================*/


class Connection{
	protected : int connID;

	public: Connection(int);
		~Connection();
		bool invia(char*);
		bool inviaRaw(void*,int);
		char* ricevi();
		char* riceviRaw(int*);
};
Connection::Connection(int sock_id){
	this->connID=sock_id;
}

Connection::~Connection(){//se la connessione è del client uso il socket,altrimenti uso il connID(facendo shutdown)
}
bool Connection::invia(char* msg){
	return this->inviaRaw(msg,strlen(msg)+1);//così invio anche il carattere di fine stringa
}
bool Connection::inviaRaw(void* rawData,int length){
	int ret=send(this->connID,rawData,length,0);
	return (ret!=length);
}
char* Connection::ricevi(){
	int length;
	char* p =(char*) riceviRaw(&length);
	*(p+length)='\0';
	return p;
}
char* Connection::riceviRaw(int* length){
	char buffer[MAX_MESSAGE+1]; //void buffer non sa quanto dimensionare ,char è un byte
	*length=recv(this->connID,buffer,MAX_MESSAGE,0);
	if(!(*length))
		printf("Error receiving buffer: %s", strerror(errno));
	
	char* ret=(char*)malloc(*length + 1);
	for(int i=0;i<=*length;i++){
		*(ret+i)=buffer[i];
	}
	return ret;
	//memcpy ( ret, buffer, le );
	//return (void*)buffer;
}
/*==================================================================*/
class ConnClient:public Connection{

	public: ConnClient(int);
		~ConnClient();
};

ConnClient::ConnClient(int conn_id):Connection(conn_id){}
ConnClient::~ConnClient(){}
/*==================================================================*/
class ConnServer:public Connection{
	public: ConnServer(int);
		~ConnServer();
};
ConnServer::ConnServer(int conn_id):Connection(conn_id){}

ConnServer::~ConnServer(){
	shutdown(connID,SHUT_RDWR);
}

/*==================================================================*/


class ServerTCP:public SocketTCP{
	private:	list<ConnServer*> connections;//chiama il costruttore
	public:		ServerTCP(int port);
			~ServerTCP();
		ConnServer* accetta();
		void disconnetti(ConnServer*);
		bool inviaTutti(char*);
		//bool inviaATuttiRaw(char*,int);
	private:void chiudiConnessioni();
};
ServerTCP::ServerTCP(int port):SocketTCP(){
	printf("Faccio la bind...");
	struct sockaddr_in myself;
	Address address(strdup(IP_LO),port);
	myself=address.getBinary();

	int ret=bind(sock_id,(struct sockaddr*) &myself,(socklen_t)sizeof(struct sockaddr_in));
	
	if (ret == -1)
        printf("Error doing bind: %s", strerror(errno));
	
	listen(sock_id,MAX_CONN);
	printf("OK\n");

}

ServerTCP::~ServerTCP(){
	chiudiConnessioni();
}

ConnServer* ServerTCP::accetta(){
	ConnServer* ret;
	int conn_id;
	struct sockaddr client;
	int len_addr=sizeof(struct sockaddr);
	conn_id=accept(sock_id,(struct sockaddr*)&client,(socklen_t*)&len_addr);
	ret=new ConnServer(conn_id);
	connections.push_front(ret);
	return ret;
}

void ServerTCP::disconnetti(ConnServer* conn){
	delete(conn);
	connections.remove(conn);
}

void ServerTCP::chiudiConnessioni(){
	while(!connections.empty()){
		disconnetti(connections.front());
	}
}

/*bool ServerTCP::inviaTutti(char* msg){
	return inviaATuttiRaw(msg,strlen(msg)+1);
}*/
/*bool ServerTCP::inviaATuttiRaw(char* msg,int len){
	bool ret=true;
	for(list<ConnServer*>::iterator it=connections.begin();ret && it!=connections.end();it++){
		ret=ret && it.inviaRaw(msg,len);
	}
	return ret;
}*/
/*==================================================================*/
class ClientTCP:public SocketTCP{
	private:	ConnClient* connection=NULL;
	public:	ClientTCP();
			~ClientTCP();
			bool connetti(Address);
			bool invia(char*);
			bool inviaRaw(void*,int);
			char* ricevi();
			char* riceviRaw(int*);
};
ClientTCP::ClientTCP():SocketTCP(){}

ClientTCP::~ClientTCP(){
	if(connection!= NULL){
		delete connection;
	}
}

bool ClientTCP::invia(char* msg){
	return (connection->invia(msg));
}

bool ClientTCP::inviaRaw(void* data,int length){
	return connection->inviaRaw(data,length);
}

char* ClientTCP::ricevi(){
	return connection->ricevi();
}

char* ClientTCP::riceviRaw(int* length){
	return connection->riceviRaw(length);
}

bool ClientTCP::connetti(Address serverAddress){
	printf("Mi connetto al server [%s:%d]",serverAddress.getIp(),serverAddress.getPort());
	struct sockaddr_in serverAddr=serverAddress.getBinary();
	int ret=connect(sock_id,(struct sockaddr*)&serverAddr,sizeof(struct sockaddr_in));
	if(ret==0){

		this->connection=new ConnClient(sock_id);
		return true;
	}
	
	printf("Error connecting to server: %s", strerror(errno));
	
return false;

}
#endif
