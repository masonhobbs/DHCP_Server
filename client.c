#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include "dhcp.h"
#define SERVER "129.120.151.94"
#define BUFLEN 512

void die(char * s)
{
    perror(s);
    exit(1);
}

int main(int argc, char ** argv)
{
    if(argc < 2)
    {
        printf("Invalid usage; use './client <port#>'\n");
        exit(1);
    }

    struct dhcp_pkt sent;
    struct dhcp_pkt rcvd;
    time_t t;
    srand((unsigned) time(&t));

    struct sockaddr_in si_other;
    int s, i;
    socklen_t slen=sizeof(si_other);
    char buf[BUFLEN];

    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
      perror("socket");
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(atoi(argv[1]));
    si_other.sin_addr.s_addr = inet_addr(SERVER);

    printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
    printf("=          Mason Hobbs - CSCE 3530            =\n");
    printf("=               DHCP     Client               =\n");
    printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n\n");

    while(1)
    {
      // Initial values for DHCP search
      sent.siaddr = si_other.sin_addr.s_addr;
      sent.yiaddr = 0;
      sent.tran_ID = rand() % 300;
      sent.lifetime = 0;

      printf("\n\n[Sending]:\n");
      print_dhcp(sent);
      if(sendto(s, &sent, sizeof(sent), 0, (struct sockaddr *)&si_other, slen) < 0)
        perror("sendto");

      if(recvfrom(s, &rcvd, sizeof(rcvd), 0, (struct sockaddr *)&si_other, &slen) < 0)
        perror("recvfrom");
      printf("\n\n[Received]: \n");
      print_dhcp(rcvd);

      // Accept IP offer
      sent.siaddr = si_other.sin_addr.s_addr;
      sent.yiaddr = rcvd.yiaddr;
      sent.tran_ID = rcvd.tran_ID + 1;
      sent.lifetime = rcvd.lifetime;
      printf("\n\n[Sending]:\n");
      print_dhcp(sent);
      if(sendto(s, &sent, sizeof(sent), 0, (struct sockaddr *)&si_other, slen) < 0)
        perror("sendto");

      // Receive ACK
      if(recvfrom(s, &rcvd, sizeof(rcvd), 0, (struct sockaddr *)&si_other, &slen) < 0)
        perror("recvfrom");
      printf("\n\n[Received]: \n");
      print_dhcp(rcvd);

      printf("\nEnter anything to quit: \n");
      char quit[256];
      fgets(quit, 256, stdin);
      exit(1);

    }
}
