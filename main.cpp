// Cezary Miłek 339746

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "icmp_utils.h"
#include "traceroute_utils.h"
#include <vector>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <destination IP>" << std::endl;
        return EXIT_FAILURE;
    }

    const char *dest_ip = argv[1];
    sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));

    if (inet_pton(AF_INET, dest_ip, &dest_addr.sin_addr) != 1)
    {
        std::cerr << "Invalid IP address: " << dest_ip << std::endl;
        return EXIT_FAILURE;
    }

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0)
    {
        perror("socket error");
        return EXIT_FAILURE;
    }

    int pid = getpid() & 0xFFFF;

    // Dla TTL od 1 do 30
    for (int ttl = 1; ttl <= 30; ++ttl)
    {
        if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0)
        {
            perror("setsockopt error");
            close(sockfd);
            return EXIT_FAILURE;
        }

        timeval times[3];
        if (!sendRequests(sockfd, ttl, pid, dest_addr, times))
        {
            close(sockfd);
            return EXIT_FAILURE;
        }

        std::vector<Response> responses;
        receiveResponses(sockfd, ttl, pid, times, responses);
        printResults(ttl, responses);

        if (targetReached(responses, dest_ip))
            break;
    }

    close(sockfd);
    return EXIT_SUCCESS;
}
