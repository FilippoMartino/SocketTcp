#include "socket_tcp.hpp"

//Funzione thread
void request(void* params);

typedef struct {

		ConnServer* conn;
		ServerTcp* myself;

} Params;

int main (int argc, char const *argv[]){
  
  int port = atoi(argv[1]);
	
		// Idea di Wang, far diventare questo un SINGLETON (DP GOF)
  ServerTcp* myself = new ServerTcp(port);
  
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
				params->connection = conn;
				params->myslef = myself;
				// Alloco il thread_id
    pthread_t thread_id;
    // Creo il thread che verrà messo in coda dal gestore di processi
    pthread_create(&thread_id,
    															NULL,
    															request, // <- funzione da chiamare
    															(void*) params; //	<- struttura con dati da passare alla funzione
  
  }  
  
  delete(myself);
  
  return 0;
  
}


void* request(void* params) {
  
  // Mi preno la risposta da fornire al client
  static char* home = HOME_PAGE;
  
  // Lo si fa per il casting
  Params* p = (Params) params;
  // Salvo i valori puntati in due variabili locali
  ConnServer* conn = (ConnServer*) p->connection;
  ServerTcp* myself = (ConnServer*) p->connection;
  // elimino i parametri dallo heap
  free(p);
  // Ricevo le richieste
  conn->ricevi;
  // Invio le risposte
  conn->invia(home);
		// Disconnetto questa conn dalla lista  
  myself->disconnetti();
  // Termino il thread
  pthread_exit(NULL); 
  
}



