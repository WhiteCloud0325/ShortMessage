#include <string>
#include "commons/tcp_socket.h"
#include "commons/comm_socket.h"
#include <iostream>
using namespace std;
int main() {
    char buf[1024] = {0};
    char *pos = buf;
    /*
    *(uint32_t*)pos = htonl(2);
    pos +=4;
    *(uint8_t*)pos++ = 1;
    *(uint8_t*)pos++ = 2;
    pos +=26;
    *(uint32_t*)pos = htonl(0);                                              
    pos += 4;
    *(uint16_t*)pos = htons(6);
    pos += 2;
    *(uint8_t*)pos++ = 0x00;
    *(uint8_t*)pos++ = 0x00;
    *(uint32_t*)pos = time(NULL);
    pos += 4;
    *(uint8_t*)pos++ = 0x10;
    *(float*)pos = 100.1;
    pos += 4;
    *(float*)pos = 32.8;
    pos += 4;
    *(float*)pos = 1000.2;
    pos += 4;
    *(uint8_t*)pos++ = 0x00;
    *(uint8_t*)pos++ = 2;
    *(uint8_t*)pos++ = 1;
    *(uint8_t*)pos++ = 1;
    *(float*)pos = 2;
    pos += 4;
    *(uint8_t*)pos++ = 1;
    *(uint8_t*)pos++ = 2;
    *(float*)pos = 3;
    pos += 4;
    */
    //message
    uint16_t frame_id = 10;
    *(uint32_t*)pos = htonl(1);
    pos += 4;
    *(uint8_t*)pos++ = 1;
    *(uint8_t*)pos++ = 2;
    pos += 26;
    *(uint32_t*)pos = htonl(2);
    pos += 4;
    *(uint16_t*)pos = htons(frame_id);
    pos +=2;
    *(uint8_t*)pos++ = 0x60;
    *(uint8_t*)pos++ = 0;
    char str[] = "adfaasfsd";
    int len = strlen(str);
    memcpy(pos, str, len);
    pos += len;
    
    char res_buf[1024] = {0};
    char *pos2 = res_buf;
    *(uint32_t*) pos2 = htonl(2);
    pos2 += 4;
    *(uint8_t*) pos2++ = 1;
    *(uint8_t*) pos2++ = 1;
    pos2 +=26;
    *(uint32_t*) pos2 = htonl(1);
    pos2 += 4;
    *(uint16_t*) pos2  = htons(frame_id);
    pos2 += 2;
    *(uint8_t*) pos2++= 0x80;
    *(uint8_t*) pos2++= 0;
    *(uint16_t*) pos2 = htons(frame_id);
    pos2 +=2;
    *(uint8_t*) pos2++ = 0x60;
    *(uint8_t*) pos2++ = 0; 
    
    TcpSocket tcp_socket;
    tcp_socket.Create();
    tcp_socket.Connect("162.105.85.118", 12000);
    tcp_socket.SendPacket(buf, pos - buf);
    int a;
    while (std::cin >> a) {
        tcp_socket.SendPacket(res_buf, pos2 - res_buf);
    }

    return 0;
}
