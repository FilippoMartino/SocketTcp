#include "socket_tcp"

#define "HOME_PAGE "HTTP/1.1 200 OK\n\
Date"

int main(){
  
  int port = atoi(argv[1]);
  char* home = HOME_PAGE;
  
  ServerTCP* myself = new ServerTCP(port);
  
  while(true){
    
    ConnServer* conn = myself->accetta();
    conn->invia(home);
    myself->disconnetti(conn);  
    
  }
  
  delete(myself);
  
  return 0;


}
