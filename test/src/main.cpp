#include <string>
#include "commons/tcp_socket.h"
#include "commons/comm_socket.h"
#include <iostream>

using namespace std;

int main() {
    char buf[1024] = {0};
    char *pos = buf;
    *(uint32_t*)pos = htonl(1);
    pos +=4;
    *(uint8_t*)pos++ = 1;
    *(uint8_t*)pos++ = 1;
    pos +=26;
    *(uint32_t*)pos = htonl(0);                                              
    pos += 4;
    *(uint16_t*)pos = htons(5);
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
    *(uint8_t*)pos++ = 2;
    *(float*)pos = 2.2;
    pos += 4;
    *(uint8_t*)pos++ = 5;
    *(uint8_t*)pos++ = 9;
    *(float*)pos = 4.4;
    pos += 4;

    TcpSocket tcp_socket;
    tcp_socket.Create();
    tcp_socket.Connect("162.105.85.118", 12000);
    tcp_socket.SendPacket(buf, pos - buf);
    char recv_buf[1024] = {0};
    int recv_len = 0;
    tcp_socket.RecvPacket(recv_buf, &recv_len);
    if (recv_len) {
        char* read_pos = recv_buf;
        uint32_t uid = *(uint32_t*)read_pos;
        read_pos += 32;
        read_pos += 6;
        uint8_t type = (*uint8_t*)read_pos;
        cout << uid << endl;
        cout << (int)type << endl;
    }
    return 0;
}