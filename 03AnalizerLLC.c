#include <stdio.h>
#define MASK 0xfe

struct entry
{
    unsigned char value;
    char protocol[100];
};

struct entry SAP[27] = {
    {0x00, "Null LSAP"},
    {0x02, "Individual LCC Sublayer Managment Function"},
    {0x03, "Group LCC Sublayer Managment Function"},
    {0x04, "IBM SNA Path Control (Individual)"},
    {0x05, "IBM SNA Path Control (Group)"},
    {0x06, "ARPANET Internet Protocol (IP)"},
    {0x08, "SNA"},
    {0x0c, "SNA"},
    {0x0e, "PROWAY (IEC955) Network Managment & Initialization"},
    {0x18, "Texas Instruments"},
    {0x42, "IEEE 802.1 Bridge Spanning Tree Protocol"},
    {0x4e, "EIA RS-511 Manufacturing Message Service"},
    {0x7e, "ISO 8208 (X.25 over IEEE 802.2 Type 2 LLC)"},
    {0x80, "Xerox Network System (XNS)"},
    {0x86, "Nestar"},
    {0x8e, "PROWAY (IEC 955) Active Station List Maintenance"},
    {0x98, "ARPANET Address Resolution Protocol (ARP)"},
    {0xbc, "Banyan VINES"},
    {0xaa, "SubNetwork Access Protocol (SNAP)"},
    {0xe0, "Novell NetWare"},
    {0xf0, "IBM NetBIOS"},
    {0xf4, "IBM LAN Management (individual)"},
    {0xf5, "IBM LAN Management (group)"},
    {0xf8, "IBM Remote Program Load (RPL)"},
    {0xfa, "Ungermann-Bass"},
    {0xfe, "ISO Network Layer Protocol"},
    {0xff, "Global LSAP"}};

struct entry NUM[21] = {
    {0x09, "SNRM"},
    {0xdf, "SNRME"},
    {0x1f, "SARM"},
    {0x5f, "SARME"},
    {0x3f, "SABM"},
    {0x7f, "SABME"},
    {0x17, "SIM"},
    {0x53, "DISC"},
    {0x73, "UA"},
    {0x1f, "DM"},
    {0x93, "RD"},
    {0x17, "RIM"},
    {0x03, "UI"},
    {0x13, "UI"},
    {0x33, "UP"},
    {0x9f, "RSET"},
    {0xaf, "XID"},
    {0xbf, "XID"},
    {0xe3, "TEST"},
    {0xf3, "TEST"},
    {0x97, "FRMR"}};

int getFrame(FILE *);
void getMacHeader(unsigned char *);
void getLLCHeader(unsigned char *);
char *getSap(unsigned char);
int isNthBitSet(unsigned char, int);
int asciiToHex(char);
char *lastBitDSAP(unsigned char);
char *lastBitSSAP(unsigned char);
char *lastBitNR(unsigned char, int);
void getInfo(unsigned char, unsigned char, int);
void getSup(unsigned char, unsigned char, int);
void getNoNum(unsigned char);
char *getSS(int, int);
int frameCounter = 1;

int main()
{
    FILE *file;
    int frameCounter = 0;
    file = fopen("tramas.txt", "r");
    if (file == NULL)
    {
        perror("Error al abrir archivo");
    }
    else
    {
        while (getFrame(file));
        fclose(file);
    }
    return 0;
}

int getFrame(FILE *file)
{
    unsigned char frame[1514];
    unsigned char c1, c2;
    int i = 0, eof = 1;
    while (1)
    {
        c1 = asciiToHex(fgetc(file));
        if (c1 == '\n' || feof(file))
        {
            if (feof(file))
                eof = 0;
            break;
        }
        c2 = asciiToHex(fgetc(file));
        frame[i] = c1 << 4 | c2;
        i++;
    }
    printf("\nTrama %d\n", frameCounter);
    frameCounter++;
    getMacHeader(frame);
    getLLCHeader(frame);
    return eof;
}

void getMacHeader(unsigned char *frame)
{
    char macDD[18];
    char macOR[18];
    int length = frame[13] + (frame[12] << 8); //2 hex to int

    snprintf(macDD, sizeof(macDD), "%02x:%02x:%02x:%02x:%02x:%02x", frame[0], frame[1], frame[2], frame[3], frame[4], frame[5]);
    snprintf(macOR, sizeof(macOR), "%02x:%02x:%02x:%02x:%02x:%02x", frame[6], frame[7], frame[8], frame[9], frame[10], frame[11]);

    printf("MAC destino: %s\n", macDD);
    printf("MAC origen: %s\n", macOR);
    printf("Longitud: %d\n", length);
    return;
}

void getLLCHeader(unsigned char *frame)
{
    int cr = isNthBitSet(frame[15], 7);// command | response
    unsigned char dsap = MASK & frame[14]; //apply mask
    unsigned char ssap = MASK & frame[15];
    printf("DSAP: %s (%s)\n", getSap(dsap), lastBitDSAP(frame[14]));//individual (0) | group (1)
    printf("SSAP: %s (%s)\n", getSap(ssap), lastBitSSAP(frame[15]));//command (0) | response (1)
    //select frame
    if (isNthBitSet(frame[16], 7))
    {
        if (isNthBitSet(frame[16], 6)) //...11
        {
            getNoNum(frame[16]);
        }
        else //...01
        {
            getSup(frame[16], frame[17], cr);
        }
    }
    else //...0
    {
        getInfo(frame[16], frame[17], cr);
    }
    return;
}

void getNoNum(unsigned char value)
{
    printf("Trama no numerada\n");
    for (int i = 0; i < 18; i++)
    {
        if (NUM[i].value == value)
        {
            printf("%s\n\n", NUM[i].protocol);
            break;
        }
    }
    return;
}

void getSup(unsigned char ns, unsigned char nr, int cr)
{
    int receiveFrame = nr / 2;
    printf("Trama de supervision\n%s, N(R): %d(%s)\n\n", getSS(isNthBitSet(ns, 4), isNthBitSet(ns, 5)), receiveFrame, lastBitNR(nr, cr));
    return;
}

void getInfo(unsigned char ns, unsigned char nr, int cr)
{
    int sendFrame = ns / 2;
    int receiveFrame = nr / 2;
    printf("Trama de informacion\nN(S): %d\nN(R): %d(%s)\n\n", sendFrame, receiveFrame, lastBitNR(nr, cr));
    return;
}


char *getSS(int bit0, int bit1) //bites SS
{
    if (bit0)
    {
        if (bit1)
            return "Selective Reject (SREJ)";//11
        return "Receive Not Ready (RNR)";//10
    }
    else
    {
        if (bit1)
            return "Reject (REJ)";//01
        return "Receive Ready (RR)";//00
    }
}

char *getSap(unsigned char value)
{
    for (int i = 0; i < 27; i++)
    {
        if (SAP[i].value == value)
            return SAP[i].protocol;
    }
    return "NA";
}

char *lastBitNR(unsigned char nr, int cr)
{
    if (isNthBitSet(nr, 7))//p/f
    {
        if (cr) //response
            return "Final";
        return "Poll"; //command
    }
    return "NA";
}

char *lastBitDSAP(unsigned char value)
{
    if (isNthBitSet(value, 7))
        return "Grupo";
    return "Individual";
}

char *lastBitSSAP(unsigned char value)
{
    if (isNthBitSet(value, 7))
        return "Respuesta";
    return "Comando";
}

int isNthBitSet(unsigned char c, int n)
{
    static unsigned char mask[] = {128, 64, 32, 16, 8, 4, 2, 1};
    return ((c & mask[n]) != 0);
}

int asciiToHex(char c)
{
    int num = (int)c;
    if (num < 58 && num > 47)
        return num - 48;
    if (num < 103 && num > 96)
        return num - 87;
    return num;
}