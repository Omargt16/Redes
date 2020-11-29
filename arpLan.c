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
#include <mysql/mysql.h>
#include <sys/time.h>

struct sockaddr_in ip_dest;
unsigned char mac_or[6];
unsigned char ip_or[4];
unsigned char mac_dest[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char mask_red[4];
unsigned char hw_type[2] = {0x00, 0x01};
unsigned char protocol[2] = {0x08, 0x00};
unsigned char mac_size[1] = {0x06};
unsigned char ip_size[1] = {0x04};
unsigned char sendOpc[2] = {0x00, 0x01};
unsigned char getOpc[2] = {0x00, 0x02};
unsigned char plotOut[1514], plotIn[1514];
unsigned char ethertype[2] = {0x08, 0x06};
unsigned char mac_broad[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
int i;
MYSQL *conn;

void getData(int);
void structPlot(unsigned char *);
void sendPlot(int, int, unsigned char *);
void getPlot(int, unsigned char *);
void queryInsert(unsigned char *);
void queryDelete();
void initDB();
int validate(unsigned char *);

int main(void)
{
    int packet_socket;
    char host[4];
    packet_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    initDB();
    queryDelete(); //remove content
    if (packet_socket == -1)
    {
        perror("Could not open socket\n");
        exit(0);
    }
    else
    {
        getData(packet_socket);
        for (int k = 0; k < 256; k++)//64
        {
            char ip_cad[20] = "192.168.1.";
            sprintf(host, "%d", k);
            strcat(ip_cad, host);
            printf("Destionation IP: %s\n", ip_cad);
            inet_pton(AF_INET, ip_cad, &(ip_dest.sin_addr));
            structPlot(plotOut);
            sendPlot(packet_socket, i, plotOut);
            getPlot(packet_socket, plotIn);
        }
        mysql_close(conn);
    }
    close(packet_socket);
    return 0;
}

void initDB()
{
    if ((conn = mysql_init(NULL)) == NULL)
    {
        fprintf(stderr, "Could not init DB\n");
        return exit(0);
    }
    if (mysql_real_connect(conn, "localhost", "omar", "Omardanielgt00#out", "arp", 0, NULL, 0) == NULL)
    {
        fprintf(stderr, "DB Connection Error\n");
        return exit(0);
    }
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
    else
    {
        perror("Plot sent");
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
    memcpy(plot + 38, &ip_dest.sin_addr.s_addr, 4);
}

void getData(int ds)
{
    struct ifreq nic;
    char name[20];
    printf("Enter interface's name: \n");
    gets(name);
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
    int tam, received = 0;
    struct timeval start, end;
    long mtime = 0, seconds, useconds;
    gettimeofday(&start, NULL);
    while (mtime < 5000) //ms
    {
        tam = recvfrom(ds, plot, 1514, 0, NULL, 0);
        if (tam == -1)
        {
            perror("\nError: Could not get plot");
        }
        else
        {
            if (validate(plot))
            {
                printf("Plot received\n");
                queryInsert(plot);
                received = 1;
                break;
            }
        }
        gettimeofday(&end, NULL);
        seconds = end.tv_sec - start.tv_sec;
        useconds = end.tv_usec - start.tv_usec;
        mtime = ((seconds)*1000 + useconds / 1000.0) + 0.5;
    }
    if (!received)
        printf("Not response\n");
    printf("Time: %ldms\n\n", mtime);
}

int validate(unsigned char *plot)
{
    if (!memcmp(plot + 0, mac_or, 6) && !memcmp(plot + 20, getOpc, 2) && !memcmp(plot + 12, ethertype, 2) && !memcmp(plot + 28, &ip_dest.sin_addr.s_addr, 4))
        return 1;
    else
        return 0;
}

void queryDelete()
{
    char deleteCache[100] = "DELETE FROM arpCache;";

    if (mysql_query(conn, deleteCache) != 0)
    {
        fprintf(stderr, "Query Failure\n");
        return exit(0);
    }
    return;
}

void queryInsert(unsigned char *plot)
{
    int array[6];
    char mac[18];
    char ip[16];
    char insertInCache[500] = "INSERT INTO arpCache (ip,mac) VALUES ('";

    inet_ntop(AF_INET, &(ip_dest.sin_addr), ip, 16);
    for (int i = 0; i < 6; i++)
        array[i] = (int)plot[i + 6];
    snprintf(mac, sizeof(mac), "%02x:%02x:%02x:%02x:%02x:%02x",
             array[0], array[1], array[2], array[3], array[4], array[5]);
    strcat(insertInCache, ip);
    strcat(insertInCache, "','");
    strcat(insertInCache, mac);
    strcat(insertInCache, "');");
    //printf("%s\n", insertInCache);
    if (mysql_query(conn, insertInCache) != 0)
    {
        fprintf(stderr, "Query Failure\n");
        return exit(0);
    }
    printf("Plot inserted\n");
    return;
}