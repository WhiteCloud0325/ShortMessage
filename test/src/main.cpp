#include <string>
#include "commons/tcp_socket.h"
#include "commons/comm_socket.h"
#include <iostream>
#include <signal.h>
#include <arpa/inet.h>
using namespace std;

bool stopped = false;
TcpSocket tcp_socket;
static void sig_int(int sig) {
    stopped = true;
    tcp_socket.Close();
    printf("service stopping...\n");
}

int main() {
    signal(SIGINT, sig_int);
    signal(SIGQUIT, sig_int);
    signal(SIGTERM, sig_int);

    /*char buf[2048] = {0};
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
    pos += 4;*/

    tcp_socket.Create();
    tcp_socket.Connect("162.105.85.118", 12002);
    char recv_buf[1024] = {0};
    int recv_len = 0;
    while(!stopped) {
        int complete_packet_num = tcp_socket.RecvPacket(recv_buf, &recv_len);
        if (complete_packet_num < 0){ //receive failed or not receive 1 or more complete packet
            tcp_socket.Close();
            break;
        }
        else{
            //std::cout << "package num: " << complete_packet_num << std::endl; 
            char* read_pos = recv_buf;
            for (int i = 0; i < complete_packet_num; i++){//get all complete packets in buffer
                int packet_len = *(int*)read_pos;
                read_pos += 4;
                //TODO
                uint32_t uid = *(uint32_t*)read_pos;
                uid = ntohl(uid);
               /* uint32_t tid = *(uint32_t*)(read_pos + 32);
                tid = ntohl(tid);
                uint16_t fid = *(uint16_t*)(read_pos + 36);
                fid = ntohs(fid);
                uint8_t type = *(uint8_t*)(read_pos + 38);*/
                uint32_t tid = *(uint32_t*)(read_pos + 4);
                tid = ntohl(tid);
                uint16_t fid = *(uint16_t*)(read_pos + 8);
                fid = ntohs(fid);
                uint8_t type = *(uint8_t*)(read_pos + 10);
                //cout << uid << endl;
                //cout << tid << endl;
                //cout << fid << endl;
                //cout << (int)type << endl;
                read_pos += packet_len;
            }
        }
    }
    return 0;
}
