#include <stdlib.h>
#include <stdint.h>
#include <netdb.h> // needed for socket(), getprotobyname() and [struct protoent]
//These are included in netdb.h:
//  <sys/socket.h>
//  <netinet/in.h>
#include "api.h"
#include "tcp_manager.h"

/*
Beschreibung
  accept() versucht einen TCP port für die ipaddress zu öffnen.
Rückgabewert
  Ist der Aufruf erfolgreich wird ein nicht-negativer Wert zurückgegeben, der die
  Verbindung identifiziert. Dieser Wert wird File Descriptor genannt.
  Im Fehlerfall wird -1 zurückgegeben.
*/
int nyx_accept(uint16_t port, uint32_t ipaddress) {

    // make new raw socket
    struct protoent* protocol_tcp = (struct protoent*) getprotobyname("TCP");
    int new_raw_fd = socket(AF_INET, SOCK_RAW, protocol_tcp->p_proto);
    free(protocol_tcp);

    // remember this particular connection in our TCP state
    tcp_manager_register(new_raw_fd, ipaddress, port);

    // initiate three-way handshake
}

/*
Beschreibung
  read() liest bis zu count Bytes vom File Descriptor fd in the Puffer der bei Adresse buf
  beginnt.
Rückgabewert
  Ist der Aufruf erfolgreich, wird die Anzahl der gelesenen Bytes zurückgegeben.
  Werden Null Bytes zurückgegeben wurde die Verbindung beendet. Es ist kein Fehler
  falls weniger als die angefragten Bytes gelesen wurden. Dies kann passieren wenn die
  Gegenseite weniger Bytes versendet hat als in den Puffer passen.
  Im Fehlerfall wird -1 zurückgegeben.
*/
ssize_t nyx_read(int fd, void* buf, size_t count) {

}

/*
Beschreibung
  write() schreibt count Bytes in den File Descriptor fd aus dem Puffer der bei Adresse
  buf beginnt.
Rückgabewert
  Ist der Aufruf erfolgreich, wird die Anzahl der gesendeten Bytes zurückgegeben.
  Im Fehlerfall wird -1 zurückgegeben.
*/
ssize_t nyx_write(int fd, void* buf, size_t count) {

}

/*
Beschreibung
  close() schließt die Verbindung, die durch den File Descriptor fd beschrieben ist. Nach
  dem Aufwurf von close() darf die Funktion write() nicht länger aufgerufen werden.
  Read()-Aufrufe können so lange erfolgen, bis auch die Gegenseite die Verbindung
  beendet hat.
Rückgabewert
  Keiner.
*/
void nyx_close(int fd) {

}
