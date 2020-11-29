#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>

unsigned char mac_or[6];
unsigned char ip_or[4];
unsigned char mac_dest[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char ip_dest[4];
unsigned char mask_red[4];
unsigned char hw_type[2] = {0x00, 0x01};
unsigned char protocol[2] = {0x08, 0x00};
unsigned char mac_size[1] = {0x06};
unsigned char ip_size[1] = {0x04};
unsigned char sendOpc[2] = {0x00, 0x01};
unsigned char getOpc[2] = {0x00, 0x02};
unsigned char plotOut[1514], plotIn[1514];
unsigned char ethertype[2] = {0x08, 0x06}; //CHANGE
unsigned char mac_broad[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
int i;

void getData(int);
void structPlot(unsigned char *);
void sendPlot(int, int, unsigned char *);
void getPlot(int, unsigned char *);

int main(void)
{
    int packet_socket;
    packet_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (packet_socket == -1)
    {
        perror("Could not open socket\n");
        exit(0);
    }
    else
    {
        getData(packet_socket);
        structPlot(plotOut);
        sendPlot(packet_socket, i, plotOut);
        getPlot(packet_socket, plotIn);
    }
    close(packet_socket);
    return 0;
}

void sendPlot(int packet_socket, int i, unsigned char *plotOut)
{
    int tam;
    struct sockaddr_ll interface;
    memset(&interface, 0x00, sizeof(interface));
    interface.sll_family = AF_PACKET;
    interface.sll_protocol = htons(ETH_P_ALL);
    interface.sll_ifindex = i;
    tam = sendto(packet_socket, plotOut, 60, 0, (struct sockaddr *)&interface, sizeof(interface));
    if (tam == -1)
    {
        perror("Could not send plot\n");
        exit(0);
    }
}

void structPlot(unsigned char *plot)
{
    //MAC header
    memcpy(plot + 0, mac_broad, 6);
    memcpy(plot + 6, mac_or, 6);
    memcpy(plot + 12, ethertype, 2);
    //ARP message
    memcpy(plot + 14, hw_type, 2);
    memcpy(plot + 16, protocol, 2);
    memcpy(plot + 18, mac_size, 1);
    memcpy(plot + 19, ip_size, 1);
    memcpy(plot + 20, sendOpc, 2);
    memcpy(plot + 22, mac_or, 6);
    memcpy(plot + 28, ip_or, 4);
    memcpy(plot + 32, mac_dest, 6);
    memcpy(plot + 38, ip_dest, 4);
}

void getData(int ds)
{
    struct ifreq nic;
    char ip_cad[20], name[20];
    printf("Enter interface's name: ");
    gets(name);
    printf("Enter destination IP: ");
    gets(ip_cad);
    inet_aton(ip_cad, ip_dest);
    strcpy(nic.ifr_name, name);
    if (ioctl(ds, SIOCGIFINDEX, &nic) == -1)
    {
        perror("Could not get index\n");
        exit(0);
    }
    else
    {
        i = nic.ifr_ifindex;
        if (ioctl(ds, SIOCGIFHWADDR, &nic) == -1)
        {
            perror("Could not get MAC\n");
            exit(0);
        }
        else
        {
            memcpy(mac_or, nic.ifr_hwaddr.sa_data, 6);
            if (ioctl(ds, SIOCGIFADDR, &nic) == -1)
            {
                perror("Could not get IP\n");
            }
            else
            {
                memcpy(ip_or, nic.ifr_addr.sa_data + 2, 4);
            }
        }
    }
}

void getPlot(int ds, unsigned char *plot)
{
    int tam;
    int array[6];
    char mac[18];
    while (1)
    {
        tam = recvfrom(ds, plot, 1514, 0, NULL, 0);
        if (tam == -1)
        {
            perror("\nCould not get plot");
            exit(0);
        }
        else
        {
            if (!memcmp(plot + 0, mac_or, 6) && !memcmp(plot + 20, getOpc, 2) && !memcmp(plot + 12, ethertype, 2) && !memcmp(plot + 28, ip_dest, 4))
            {
                for (int i = 0; i < 6; i++)
                    array[i] = (int)plot[i + 6];
                snprintf(mac, sizeof(mac), "%02x:%02x:%02x:%02x:%02x:%02x",
                         array[0], array[1], array[2], array[3], array[4], array[5]);

                printf("The MAC address is: %s", mac);
                printf("\n");
                break;
            }
        }
    }
}