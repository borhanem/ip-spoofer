#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/random.h>
#include <net/ethernet.h> /* the L2 protocols */
#include <linux/if_packet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <features.h>

/*
1000   --   minimize delay
0100   --   maximize throughput
0010   --   maximize reliability
0001   --   minimize monetary cost
0000   --   normal service
*/

#define TOT_ARG 5
#define MAX_STR_LEN 1024

int check_addr_format(char** addr){
    int values[4];
    if(sscanf(*addr,"%d.%d.%d.%d",&values[0],&values[1],&values[2],&values[3]) != 4){
        printf("invalid address: %s\n",*addr);
        return 0;
    }
    for (size_t i = 0; i < 4; i++)
    {
        if(values[i] < 0 || 255 < values[i]){
            printf("Invalid address: %s\n",*addr);
            return 0;
        }
    }
    return 1;
}


int main(int argc, char **argv)
{
    if (argc != TOT_ARG)
    {
        printf("Usage: ./raw <src_ip> <dst_ip> <src_port> <dst_port>\n");
        exit(-1);
    }else if(!(check_addr_format(&argv[1]),check_addr_format(&argv[2]))){
        exit(-1);
    }
    char src_addr[16], dst_addr[16];
    strcpy(src_addr, argv[1]);
    strcpy(dst_addr, argv[2]);
    int src_port = atoi(argv[3]), dst_port = atoi(argv[4]);
    int sockfd;
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sockfd < 0)
    {
        perror("ERROR IN SOCKET\n");
    }
    char data[MAX_STR_LEN] = "hello";

    struct ip ip_header;
    ip_header.ip_hl = sizeof(struct ip) / 4;
    ip_header.ip_v = 4;
    ip_header.ip_tos = 4;
    ip_header.ip_len = htons(sizeof(struct ip) + sizeof(struct udphdr) + sizeof(data));
    ip_header.ip_id = 0;
    ip_header.ip_off = 0;
    ip_header.ip_ttl = IPDEFTTL;
    ip_header.ip_p = IPPROTO_UDP;
    ip_header.ip_sum = 0;

    struct in_addr src_ip;
    src_ip.s_addr = inet_addr(src_addr);
    ip_header.ip_src = src_ip;

    struct in_addr dst_ip;
    dst_ip.s_addr = inet_addr(dst_addr);
    ip_header.ip_dst = dst_ip;

    struct udphdr udp_header;
    udp_header.uh_sport = htons(src_port);                            // Source port.
    udp_header.uh_dport = htons(dst_port);                            // Destination port.
    udp_header.uh_ulen = htons(sizeof(struct udphdr) + sizeof(data)); // Length of data + udp header length.
    udp_header.uh_sum = 0;

    int datagram_size = sizeof(struct ip) + sizeof(struct udphdr) + sizeof(data);
    unsigned char datagram[datagram_size];
    memcpy(datagram, &ip_header, sizeof(struct ip));
    memcpy(datagram + sizeof(struct ip), &udp_header, sizeof(struct udphdr));

    struct sockaddr_in destaddr;
    destaddr.sin_family = AF_INET;
    destaddr.sin_port = htons(80);
    destaddr.sin_addr.s_addr = inet_addr(dst_addr);

    // PACKET SENDING STARTS HERE
    if (sendto(sockfd, datagram, datagram_size, 0, (struct sockaddr *)&destaddr, sizeof(destaddr)) == -1)
    {
        perror("Couldn't send packet");
    }
}