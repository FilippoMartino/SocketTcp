#include "socket_tcp"

void request(void* connection);

int main (int argc, char const *argv[]){
  
  int port = atoi(argv[1]);
  char* home = HOME_PAGE;
  
  ServerTcp* myself = new ServerTcp(port);
  
  while(true){
    
    ConnServer* conn = myself->accetta();
    pthread_t
  
  }  
  
}


void* request(void connection) {
  
  ConnServer * conn = (ConnServer*) connection;
  conn->invia(HOME_PAGE);
  
}
