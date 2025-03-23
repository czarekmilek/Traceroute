// Cezary Miłek 339746

#ifndef TRACEROUTE_UTILS_H
#define TRACEROUTE_UTILS_H

#include <vector>
#include <netinet/in.h>
#include "icmp_utils.h"

// Wysyła 3 pakiety ICMP echo request dla danego TTL.
// Zapisuje czasy wysłania w tablicy `times`.
// Zwraca true, jeśli wysłanie przebiegło pomyślnie.
bool sendRequests(int sockfd, int ttl, int pid, const sockaddr_in &dest, timeval times[3]);

// Odbiera odpowiedzi przez maksymalnie 1 sekundę dla danego TTL.
// Odebrane odpowiedzi są dodawane do wektora `responses`.
void receiveResponses(int sockfd, int ttl, int pid, const timeval times[3], std::vector<Response> &responses);

// Wypisuje wynik dla danego TTL: numer TTL, unikalne adresy oraz średni czas RTT (lub "???").
void printResults(int ttl, const std::vector<Response> &responses);

// Sprawdza, czy wśród odebranych odpowiedzi znajduje się odpowiedź od docelowego adresu.
bool targetReached(const std::vector<Response> &responses, const char *dest_ip);

#endif
