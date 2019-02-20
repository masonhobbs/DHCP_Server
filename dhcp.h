#include <string.h>

void print_ip(unsigned int);
char * offer_ip(char gateway[256], char subnet_mask[256]);

#pragma pack(1)
struct dhcp_pkt{

  unsigned int siaddr; // Server IP address
  unsigned int yiaddr; // Your IP address
  unsigned int tran_ID; // Transaction ID
  unsigned short int lifetime; // Lease time of the IP address

};
#pragma pack(0)

int last_leased[4] = {-1, -1, -1, -1};

void print_dhcp(struct dhcp_pkt input)
{
  printf("<><><><><><><><><>\nDHCP Packet Info\n<><><><><><><><><>");
  printf("\n\tsiaddr: ");
  print_ip(input.siaddr);
  printf("\tyiaddr: ");
  print_ip(input.yiaddr);
  printf("\ttran_ID: %d\n", input.tran_ID);
  printf("\tlifetime: %d\n", input.lifetime);
  printf("<><><><><><><><><>\n");
}

void print_ip(unsigned int ip)
{
  unsigned char ip_addr[4];
  ip_addr[0] = ip & 0xFF;
  ip_addr[1] = (ip >> 8) & 0xFF;
  ip_addr[2] = (ip >> 16) & 0xFF;
  ip_addr[3] = (ip >> 24) & 0xFF;

  printf("%d.%d.%d.%d\n", ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);
}

char * offer_ip(char gateway_orig[256], char subnet_mask_orig[256])
{
  // Stores the IP we will lease to client after calculations
  char * offer = malloc(sizeof(char) * 256);
  char gateway[256];
  char subnet_mask[256];

  strcpy(gateway, gateway_orig);
  strcpy(subnet_mask, subnet_mask_orig);

  // Range for each octet is defined by mask_settings
  int mask_settings[4] = {0, 0, 0, 0};
  // Gives us the IP of gateway for each octet
  int gateway_settings[4] = {0, 0, 0, 0};
  // Gives us the max IP allowed to be leased
  int max_ip[4] = {0, 0, 0, 0};
  // Octets for IP addressed to be leased
  int lease_ip[4] = {0, 0, 0, 0};
  // Loop vars for tokens
  int mask_index = 0;
  int gateway_index = 0;
  // For tokenizing mask and gateway
  char * token;
  char * iptoken;

  // Take subnet mask and gateway and tokenize it, checking for values of 255 to determine the range of
  // the IP addresses we can offer

  token = strtok(subnet_mask, ".");
  mask_settings[mask_index] = atoi(token);
  mask_index++;

  while((token = strtok(NULL, ".")) != NULL) {
    mask_settings[mask_index] = atoi(token);
    mask_index++;
  }

  iptoken = strtok(gateway, ".");
  gateway_settings[gateway_index] = atoi(iptoken);
  gateway_index++;

  while((iptoken = strtok(NULL, ".")) != NULL) {
    gateway_settings[gateway_index] = atoi(iptoken);
    gateway_index++;
  }

  // Determine the max possible IP address we can offer
  // without touching off limits one...
  int i;
  int j;

  for(i = 0; i < 4; i++) {
    max_ip[i] = gateway_settings[i] + (255 - mask_settings[i]);
    if(max_ip[i] > 255)
      max_ip[i] = 255;
    if(mask_settings[i] == 0 || mask_settings[i] != 255)
      max_ip[i] -= 1;
  //  if(i != 0 && mask_settings[i-1] == 255)
    //  max_ip[i] -= 1;
    if(i != 0 && mask_settings[i-1] == 0)
      max_ip[i] += 1;
    else if(i == 0 && mask_settings[i+1] != 0)
      max_ip[i] += 1;
    if(mask_settings[i] == 255)
      max_ip[i] = gateway_settings[i];
  }

  // Create new IP address, currently only
  // handles one client's IP request

  char tmp[64];
  strcpy(offer, "");

  // If no IP addresses already assigned...
  if(last_leased[0] == -1 && last_leased[1] == -1 && last_leased[2] == -1 && last_leased[3] == -1) {
    // Simply add 1 to da gateway address
    for(i = 0; i < 4; i++) {
      last_leased[i] = gateway_settings[i];
      lease_ip[i] = gateway_settings[i];

      if(i == 3) {
        last_leased[i] += 1;
        lease_ip[i] += 1;
      }
      sprintf(tmp, "%d", lease_ip[i]);
      strcat(offer, tmp);
      if(i != 3)
        strcat(offer, ".");
    }
    return offer;
  }

  // Else if we've assigned an IP address before....
  else {
    if(last_leased[0] == max_ip[0] && last_leased[1] == max_ip[1] && last_leased[2] == max_ip[2] && last_leased[3] == max_ip[3]-1)
    {
      strcpy(offer, "No IP's left to assign.");
      return offer;
    }
    for(i = 0; i < 4; i++) {
      // For static octet
      if(mask_settings[i] == 255) {
        lease_ip[i] = gateway_settings[i];
      }
      // For mask settings not 255
      if(mask_settings[i] != 255) {
          // Find out which slot to increment by checking
          // if last_leased spot starting from end is max allowed
          if(last_leased[3] != max_ip[3]) {
              lease_ip[3] = last_leased[3] += 1;
              lease_ip[2] = gateway_settings[2];
              lease_ip[1] = gateway_settings[1];
              lease_ip[0] = gateway_settings[0];
              printf("incremented last index by 1, breaking\n");
          }
          else if(last_leased[2] != max_ip[2] && last_leased[3] == max_ip[3]) {
              lease_ip[3] = gateway_settings[3];
              lease_ip[2] = last_leased[2] += 1;
              lease_ip[1] = gateway_settings[1];
              lease_ip[0] = gateway_settings[0];

              printf("incremented 2nd to last index by 1, breaking\n");
          }
          else if(last_leased[1] != max_ip[1] && last_leased[3] == max_ip[3] && last_leased[2] == max_ip[2]) {
              lease_ip[3] = gateway_settings[3];
              lease_ip[2] = gateway_settings[2];
              lease_ip[1] = last_leased[1] += 1;
              lease_ip[0] = gateway_settings[0];
              printf("incremented 3nd to last index by 1, breaking\n");
          }

          for(j = 0; j < 4; j++) {
            last_leased[j] = lease_ip[j];
          }

          //strcat(offer, tmp);
          break;
      }
    }
  }

  for(i = 0; i < 4; i++) {
    sprintf(tmp, "%d", lease_ip[i]);
    strcat(offer, tmp);
    if(i != 3)
      strcat(offer, ".");
  }

  return offer;
}
