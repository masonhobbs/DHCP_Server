#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include "dhcp.h"
#define SERVER "129.120.151.94"
#define BUFLEN 256
//sploot
/*---------------------------------------------------*/
/*              Client-Server Handling               */
/*---------------------------------------------------*/
int main(int argc, char ** argv)
{
    if(argc < 2)
    {
        printf("Invalid usage; use './dhcp_server <port#>'\n");
        exit(1);
    }

    printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
    printf("=          Mason Hobbs - CSCE 3530            =\n");
    printf("=               DHCP     Server               =\n");
    printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n\n");
    struct sockaddr_in hold;
    struct dhcp_pkt sent;
    struct dhcp_pkt rcvd;

    char gateway[BUFLEN];
    char subnet_mask[BUFLEN];
    char * offer = malloc(sizeof(char) * 256);
    printf("Enter gateway: ");
    fgets(gateway, BUFLEN, stdin);
    printf("Enter subnet mask: ");
    fgets(subnet_mask, BUFLEN, stdin);

    struct sockaddr_in si_me, si_other;
    int s, i;
    socklen_t slen=sizeof(si_other);
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
      perror("socket");

    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(atoi(argv[1]));
    si_me.sin_addr.s_addr = inet_addr(SERVER);
    if (bind(s, (struct sockaddr *)&si_me, sizeof(si_me))==-1)
      perror("bind");

    fflush(stdout);
    printf("Waiting on a client request...\n");
    while(1)
    {
      // Receive a DHCP discover from a client
      if(recvfrom(s, &rcvd, sizeof(rcvd), 0, (struct sockaddr *)&si_other, &slen) < 0)
        perror("recvfrom");
      printf("\n\n[Received]: \n");
      print_dhcp(rcvd);

      // Generate IP offer
      strcpy(offer, offer_ip(gateway, subnet_mask));
      printf("offer: %s\n", offer);
      // Store IP
      if(inet_aton(offer, &hold.sin_addr) == 0)
      {
          fprintf(stderr, "Used all available IPs. Exiting.\n");
          exit(1);
      }

      // Send back IP to lease
      sent.siaddr = rcvd.siaddr;
      sent.yiaddr = hold.sin_addr.s_addr;
      sent.tran_ID = rcvd.tran_ID + 1;
      sent.lifetime = 3600;
      printf("\n\n[Sending]:\n");
      print_dhcp(sent);
      if(sendto(s, &sent, sizeof(rcvd), 0, (struct sockaddr *)&si_other, slen) < 0)
        perror("sendto");

      // Receive a DHCP offer acceptance from a client
      if(recvfrom(s, &rcvd, sizeof(rcvd), 0, (struct sockaddr *)&si_other, &slen) < 0)
        perror("recvfrom");
      printf("\n\n[Received]: \n");
      print_dhcp(rcvd);

      // Send ACK back
      printf("\n\n[Sending]:\n");
      print_dhcp(rcvd);
      if(sendto(s, &rcvd, sizeof(rcvd), 0, (struct sockaddr *)&si_other, slen) < 0)
        perror("sendto");

      fflush(stdout);
      printf("Client request completed. Waiting on new clients, or press CTRL+Z to quit...\n");
    }

}
