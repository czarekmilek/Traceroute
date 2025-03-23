// Cezary Miłek 339746

#ifndef ICMP_UTILS_H
#define ICMP_UTILS_H

#include <sys/time.h>
#include <cstdint>
#include <string>

// Oblicza sumę kontrolną nagłówka ICMP.
uint16_t computeChecksum(const void *buff, int length);

// Oblicza różnicę czasu między `start` a `end` w milisekundach.
double getTimeDiffMs(const timeval &start, const timeval &end);

// Prosta struktura przechowująca informacje o odpowiedzi:
// `index` – numer pakietu (0,1,2)
// `rtt` – czas RTT w ms
// `ip` – adres źródłowy
struct Response
{
  int index;
  double rtt;
  std::string ip;
};

#endif
