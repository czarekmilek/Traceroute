// Cezary Miłek 339746

#include "traceroute_utils.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <unistd.h>
#include <cstdlib>
#include <cerrno>

bool sendRequests(int sockfd, int ttl, int pid, const sockaddr_in &dest, timeval times[3])
{
  for (int i = 0; i < 3; ++i)
  {
    icmphdr icmp;
    memset(&icmp, 0, sizeof(icmp));
    icmp.type = ICMP_ECHO;        // Echo request
    icmp.un.echo.id = htons(pid); // Ustawiamy identyfikator

    // Numer sekwencyjny: wysokie 8 bitów = ttl, niskie 8 bitów = numer pakietu
    icmp.un.echo.sequence = htons((ttl << 8) | i);
    icmp.checksum = 0;
    icmp.checksum = computeChecksum(&icmp, sizeof(icmp));

    // Zapisujemy czas wysłania pakietu
    if (gettimeofday(&times[i], nullptr) < 0)
    {
      perror("gettimeofday");
      return false;
    }

    ssize_t sent = sendto(sockfd, &icmp, sizeof(icmp), 0,
                          reinterpret_cast<const sockaddr *>(&dest), sizeof(dest));
    if (sent < 0 || sent != sizeof(icmp))
    {
      perror("sendto");
      return false;
    }
  }
  return true;
}

void receiveResponses(int sockfd, int ttl, int pid, const timeval times[3], std::vector<Response> &responses)
{
  const int timeout = 1000;
  int elapsed = 0;
  timeval start;
  if (gettimeofday(&start, nullptr) < 0)
  {
    perror("gettimeofday");
    return;
  }

  while (elapsed < timeout)
  {
    pollfd pfd;
    pfd.fd = sockfd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    int left = timeout - elapsed;
    int result = poll(&pfd, 1, left);
    if (result < 0)
    {
      if (errno == EINTR)
        continue;
      perror("poll");
      break;
    }
    else if (result == 0)
    {
      break;
    }

    const int buffer_size = 1500;
    uint8_t buf[buffer_size];
    sockaddr_in src_addr;
    socklen_t src_addr_len = sizeof(src_addr);

    ssize_t recieved_bytes = recvfrom(sockfd, buf, buffer_size, 0,
                         reinterpret_cast<sockaddr *>(&src_addr), &src_addr_len);
    if (recieved_bytes < 0)
    {
      perror("recvfrom");
      break;
    }

    timeval recieve_time;
    if (gettimeofday(&recieve_time, nullptr) < 0)
    {
      perror("gettimeofday");
      break;
    }

    // Parsujemy nagłówek IP
    ip *ip_header = reinterpret_cast<ip *>(buf);
    int ip_length = ip_header->ip_hl * 4;
    if (recieved_bytes < ip_length + static_cast<int>(sizeof(icmphdr)))
      continue;

    icmphdr *icmp_response = nullptr;
    uint8_t type = *(buf + ip_length);
    if (type == ICMP_ECHOREPLY)
    {
      icmp_response = reinterpret_cast<icmphdr *>(buf + ip_length);
    }
    else if (type == ICMP_TIME_EXCEEDED)
    {
      if (recieved_bytes < ip_length + static_cast<int>(sizeof(icmphdr)) + 20 + static_cast<int>(sizeof(icmphdr)))
        continue;
      // Odczytujemy oryginalny nagłówek IP i ustalamy miejsce, gdzie znajduje się oryginalny nagłówek ICMP
      ip *original_ip = reinterpret_cast<ip *>(buf + ip_length + sizeof(icmphdr));
      int original_ip_length = original_ip->ip_hl * 4;
      icmp_response = reinterpret_cast<icmphdr *>(buf + ip_length + sizeof(icmphdr) + original_ip_length);
    }
    else
    {
      continue;
    }

    if (ntohs(icmp_response->un.echo.id) != pid)
      continue;
    int seq = ntohs(icmp_response->un.echo.sequence);
    int sent_ttl = seq >> 8;
    int index = seq & 0xFF;

    if (sent_ttl != ttl || index < 0 || index >= 3)
      continue;

    double rtt = getTimeDiffMs(times[index], recieve_time);
    char ip_str[INET_ADDRSTRLEN];

    if (inet_ntop(AF_INET, &(src_addr.sin_addr), ip_str, sizeof(ip_str)) == nullptr)
      strcpy(ip_str, "unknown");

    Response resp;
    resp.index = index;
    resp.rtt = rtt;
    resp.ip = ip_str;
    responses.push_back(resp);

    timeval now;
    if (gettimeofday(&now, nullptr) < 0)
      break;
    elapsed = static_cast<int>(getTimeDiffMs(start, now));

    if (responses.size() == 3)
          break;
  }
}

void printResults(int ttl, const std::vector<Response> &responses)
{
  std::ostringstream oss;
  oss << std::setw(3) << ttl << ". ";
  if (responses.empty())
  {
    oss << "*";
  }
  else
  {
    // Zbieramy unikalne adresy (w kolejności otrzymania)
    std::vector<std::string> unique;
    for (const auto &r : responses)
    {
      bool exists = false;
      for (const auto &s : unique)
      {
        if (s == r.ip)
        {
          exists = true;
          break;
        }
      }
      if (!exists)
        unique.push_back(r.ip);
    }

    for (const auto &s : unique)
      oss << std::left << std::setw(15) << s << " ";

    if (responses.size() == 3)
    {
      double total = 0;
      for (const auto &r : responses)
        total += r.rtt;
      double total_cut = total / 3;
      oss << std::fixed << std::setprecision(3) << total_cut << "ms";
    }
    else
    {
      oss << "???";
    }
  }
  std::cout << oss.str() << std::endl;
}

bool targetReached(const std::vector<Response> &responses, const char *dest_ip)
{
  for (const auto &r : responses)
    if (r.ip == dest_ip)
      return true;
  return false;
}
