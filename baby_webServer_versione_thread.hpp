#include "SocketTCP.hpp"

#define HOME "HTTP/1.1 200 OK\n\
Date: Wed, 19 Apr 2017 16:34:20 GMT\n\
Content-Type: text/html; charset=UTF-8\n\
\n <!DOCTYPE html>\n\
<html class=\"client-nojs\" lang=\"it\" dir=\"ltr\">\n\
<head>\n\
<title>web site</title>\n\
<body>\n\
<h1>Hello world</h1>\n\
</body>\n\
</html>"

typedef struct {

		ConnServer* conn;
		ServerTCP* myself;

} Params;

void* request(void* params) {	
  
  // Mi prendo la risposta da fornire al client
  static char* home = strdup("foo");
  
  // Lo si fa per il casting
  Params* p = (Params*) params;
  // Salvo i valori puntati in due variabili locali
  ConnServer* conn = (ConnServer*) p->conn;
  ServerTCP* myself = (ServerTCP*) p->myself;
  // elimino i parametri dallo heap
  free(p);
  // Ricevo le richieste
  conn->ricevi();
  // Invio le risposte
  conn->invia(home);
		// Disconnetto questa conn dalla lista  
  myself->disconnetti(conn);
  // Termino il threadz
  pthread_exit(NULL); 
  
}

int main (int argc, char const *argv[]){
  
  int port = atoi(argv[1]);
	
		// Idea di Wang, far diventare questo un SINGLETON (DP GOF)
  ServerTCP* myself = new ServerTCP(port);
  
  while(true){
    
    ConnServer* conn = myself->accetta();
    /*
    
    		Dobbiamo mallocare la uno spazio in memoria
    		con i parametri in modo che sia sempre visibile
    		al thread cui dovrà lavorarci, altrimenti verrebbe
    		deallocata alla fine del while.
    		Così facendo alloco uno spazio nello heap, lo faccio
    		puntare da una variabile e passo la stessa alla funzione
    		pthread_create, che la utilizzerà per il thread stesso.
    		Alla fine del ciclo il puntatore verrà deallocato, senza
    		toccare i parametri nello heap che verranno poi eliminati 
    		dal thread dopo averne prelevato i valori.
    		
    */
				Params* params = (Params*) malloc(sizeof(Params));
				// Assegno i valori all'area di memoria
				params->conn = conn;
				params->myself = myself;
				// Alloco il thread_id
    pthread_t thread_id;
    // Creo il thread che verrà messo in coda dal gestore di processi
    pthread_create(&thread_id, NULL, request,	 (void*) params); 
  
  }  
  
  delete(myself);
  
  return 0;
  
}






