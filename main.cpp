#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <fstream>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <vector>

std::mutex mtx;

struct udp_reply {
    const char * reply;
    int len;
};

struct stats {
    int seeds;
    int downloaded;
    int leechs;
    int servers;
};

char * hexstr2byte(const char * hexstr){
    int len = strlen(hexstr);
    if (len%2 == 0){
        char * buff = (char *) malloc(len / 2);
        int buff_len = 0;
        const char * pos = hexstr;

        for (int x = 0; x < len; x++){
            unsigned char *byte = (unsigned char *) malloc(1);
            sscanf(pos, "%2hhx", byte);
            memcpy(buff + buff_len, byte, 1);
            buff_len += 1;
            pos += 2;
        }

        return buff;
    } else {
        return nullptr;
    }
}

udp_reply get_connection_id(const char * ip, int port){

    udp_reply connect_reply;
    connect_reply.reply = "Error";
    connect_reply.len = -1;
    
    int socket_info = socket(AF_INET, SOCK_DGRAM, 0);
    
    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 500000;

    if (socket_info == -1){
        std::cout << "Couldn't create a socket.\n";
        return connect_reply;
    }

    sockaddr_in client;
    client.sin_addr.s_addr = inet_addr(ip);
    client.sin_family = AF_INET;
    client.sin_port = htons(port);
    
    if (connect(socket_info, (sockaddr *) &client, sizeof(client)) < 0){
        std::cout << "Error connecting.\n";
        return connect_reply;
    }

    const int64_t proto = htobe64(0x41727101980); //Constant for BitTorrent protocol
    const char protocol[] = "\x00\x00\x04\x17\x27\x10\x19\x80";
    int32_t action = htonl(0); //Connection request is zero.
    int32_t txn_id = htonl(std::rand() % 10000 + 1);

    char * buffer = (char *) malloc(16);
    int curr_size = 0;

    memcpy(buffer, protocol, 8);
    curr_size += 8;
    memcpy(buffer + curr_size, (char *) &action, sizeof(action));
    curr_size += sizeof(action);
    memcpy(buffer + curr_size, (char *) &txn_id, sizeof(txn_id));
    curr_size += sizeof(txn_id);

    if (send(socket_info, buffer, curr_size, 0) < 0){
        std::cout << "Error sending message.\n";
        return connect_reply;
    }

    char * response = (char *) malloc(1024);
    setsockopt(socket_info, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv));
    int len = recv(socket_info, response, 1024, 0);
    // std::cout << "Received.\n";
    if (len < 0){
        return connect_reply;
    } else {
        connect_reply.reply = response;
        connect_reply.len = len;
        return connect_reply;
    }

}

udp_reply scrape_tracker(const char * ip, int port, const char * connection_id, const char * infohash){
    
    udp_reply scrape_reply;
    scrape_reply.reply = "Error";
    scrape_reply.len = -1;

    int socket_info = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_info == -1){
        std::cout << "Couldn't create a socket.\n";
        return scrape_reply;
    }

    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 500000;

    sockaddr_in client;
    client.sin_addr.s_addr = inet_addr(ip);
    client.sin_family = AF_INET;
    client.sin_port = htons(port);
    
    if (connect(socket_info, (sockaddr *) &client, sizeof(client)) < 0){
        std::cout << "Error connecting.\n";
        return scrape_reply;
    } else {
        char * buffer = (char *) malloc(36); //36 bytes to send scrape request.

        int32_t action = htobe32(2);
        int32_t txn_id = htonl(std::rand() % 10000 + 1);
        
        int buff_size = 0;
        memcpy(buffer, connection_id, 8);
        buff_size += 8;
        memcpy(buffer + buff_size, (char *) &action, sizeof(action));
        buff_size += sizeof(action);
        memcpy(buffer + buff_size, (char *) &txn_id, sizeof(txn_id));
        buff_size += sizeof(txn_id);
        memcpy(buffer + buff_size, hexstr2byte(infohash), 20);
        buff_size += 20;

        if (send(socket_info, buffer, buff_size, 0) < 0){
            std::cout << "Error sending message.\n";
            return scrape_reply;
        }

        char * response = (char *) malloc(1024);
        setsockopt(socket_info, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv));
        int len = recv(socket_info, response, 1024, 0);

        if (len < 0){
            return scrape_reply;
        } else {
            scrape_reply.reply = response;
            scrape_reply.len = len;
            return scrape_reply;
        }
    }
}
void hex2ip(const char * hex){
    uint8_t ip[4];
    memcpy(ip, hex, 4); //Copy the 4 bytes to a 4 x 1byte int array.
    mtx.lock();
    for (int x = 0; x < 3; x++){
        std::cout << +ip[x] << ".";
    }

    std::cout << +ip[3] << '\n';
    mtx.unlock();
}

udp_reply announce_tracker(const char * ip, int port, const char * connection_id, const char * infohash){

    udp_reply announce_reply;
    announce_reply.reply = "Error";
    announce_reply.len = -1;

    int socket_info = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_info == -1){
        std::cout << "Couldn't create a socket.\n";
        return announce_reply;
    }

    timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 500000;

    sockaddr_in client;
    client.sin_addr.s_addr = inet_addr(ip);
    client.sin_family = AF_INET;
    client.sin_port = htons(port);

    if (connect(socket_info, (sockaddr *) &client, sizeof(client)) < 0){
        std::cout << "Error connecting.\n";
        return announce_reply;
    }

    char * buffer = (char *) malloc(98); //Buffer which will hold the hex fo packet;
    int bufflen = 0;
    int32_t action = htonl(1); //Annouce is 1
    int32_t txn_id = htonl(std::rand() % 10000 + 1);
    const char downloaded[] = "\x00\x00\x00\x00\x00\x00\x00\x00"; //8 byte downloaded value
    const char left[] = "\x00\x00\x00\x00\x00\x00\x00\x00"; //8 byte left value
    const char uploaded[] = "\x00\x00\x00\x00\x00\x00\x00\x00"; //8 byte uploaded value
    int32_t event_id = htonl(2); //We tell tracker we started.
    uint32_t client_ip = htonl(0); //Tell tracker to use packet IP, since normally not honored anyway.
    uint32_t client_key = htonl(std::rand() % 10000 + 1);
    int32_t num_want = htonl(-1); //Default peers. Can try diff values
    uint16_t client_port = htons(5000); //Set some port, doesnt really matter.
    // uint16_t extensions
    //Peerid will be randomised during memcpy.

    memcpy(buffer + bufflen, connection_id, 8); //8 byte connection id
    bufflen += 8;
    memcpy(buffer + bufflen, (char *) &action, 4); //4 byte action
    bufflen += 4;
    memcpy(buffer + bufflen, (char *) &txn_id, 4); //4 byte txn_id
    bufflen += 4;
    memcpy(buffer + bufflen, hexstr2byte(infohash), 20); //Copy the 20 bytes of infohash.
    bufflen += 20;
    memcpy(buffer + bufflen, (void *)memcpy, 20); //20 bytes for peerid
    bufflen += 20;
    memcpy(buffer + bufflen, downloaded, 8);
    bufflen += 8;
    memcpy(buffer + bufflen, left, 8);
    bufflen += 8;
    memcpy(buffer + bufflen, uploaded, 8);
    bufflen += 8;
    memcpy(buffer + bufflen, (char *) &event_id, 4); //4 bytes for the event
    bufflen += 4;
    memcpy(buffer + bufflen, (char *) &client_ip, 4); //4 bytes for ip.
    bufflen += 4;
    memcpy(buffer + bufflen, (char *) &client_key, 4); //4 bytes for random ip
    bufflen += 4;
    memcpy(buffer + bufflen, (char *) &num_want, 4);
    bufflen += 4;
    memcpy(buffer + bufflen, (char *) &port, 2);
    bufflen += 2;

    if (send(socket_info, buffer, bufflen, 0) < 0){
        std::cout << "Error sending the announce.";
        return announce_reply;
    }

    char * response = (char *) malloc(1024);
    setsockopt(socket_info, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv));
    int len = recv(socket_info, response, 1024, 0);

    if (len < 0){
        return announce_reply;
    }

    announce_reply.reply = response;
    announce_reply.len = len;

    int peers = (len - 20) / 6;

    for (int i = 0; i < peers; i++){
        if (20 + (i * 6) < 1020){
            hex2ip(response + 20 + (i * 6));
        }
    }

    return announce_reply;
}

void scrape(const char * infohash, std::string ip, int tracker_port, stats &t_stat){
    const char * tracker_ip = ip.c_str();
    udp_reply connect_reply;
    connect_reply = get_connection_id(tracker_ip, tracker_port);

    char * connection_id = (char *) malloc(8);

    if (connect_reply.len == 16){
        memcpy(connection_id, connect_reply.reply + 8, 8);
        udp_reply scrape_reply = scrape_tracker(tracker_ip, tracker_port, connection_id, infohash);
        if (scrape_reply.len == 20){
            int seeds, done, leech;

            memcpy((char *) &seeds, (char *) scrape_reply.reply+8, 4);
            t_stat.seeds += be32toh(seeds);
            memcpy((char *) &done, scrape_reply.reply+12, 4);
            t_stat.downloaded += be32toh(done);
            memcpy((char *) &leech, scrape_reply.reply+16, 4);
            t_stat.leechs += be32toh(leech);
            t_stat.servers++;
        }
    }
}

void announce(const char * infohash, std::string ip, int tracker_port){
    const char * tracker_ip = ip.c_str();
    udp_reply connect_reply = get_connection_id(tracker_ip, tracker_port);

    char * connection_id = (char *) malloc(8);

    if (connect_reply.len != 16){
        return;
    }

    memcpy(connection_id, connect_reply.reply + 8, 8);
    udp_reply announce_reply = announce_tracker(tracker_ip, tracker_port, connection_id, infohash);

}

int main(int argc, char * argv[]){
    std::srand(std::time(nullptr));
    int opt;

    int scrape_flag = 0, announce_flag = 0, nt_var = 1 , force_flag = 0;
    std::string tracker_filename = "trackers.txt";

    if (argc < 2){
        std::cerr << "Usage: ./func [40 character hash to scrape]\n";
        return -1;
    }

    if (strlen(argv[1]) != 40){
        std::cerr << "Hash is not 40 characters wrong. Exiting...\n";
        return -1;
    }

    char infohash[40];
    strcpy(infohash, argv[1]);

    while ((opt = getopt(argc, argv, "asft:r:")) != -1){
        switch (opt){
            case 'a':
                announce_flag = 1;
                break;
            case 's':
                scrape_flag = 1;
                break;
            case 't':
                tracker_filename = optarg;
                break;
            case 'f':
                force_flag = 1;
                break;
            case 'r':
                nt_var = std::stoi(optarg);
                break;
        }
    }

    if (announce_flag == 1 && scrape_flag == 1){
        std::cout << "You may only use -a (announce) OR -s (scrape) at a time.\n";
    } else if (announce_flag == 0 && scrape_flag == 0){
        scrape_flag = 1;
    }

    std::ifstream trackers(tracker_filename);
    std::string tracker;

    stats t_stat = {0, 0, 0, 0};

    const int num_threads = 19;
    const int multi_req = nt_var;
    std::vector< std::vector< std::thread > > t;
    int f = 0;

    while (trackers >> tracker){
        int pos = tracker.find(':');
        std::string ip = tracker.substr(0, pos);
        int port = stoi(tracker.substr(pos+1, tracker.length() - (pos + 1)));
        std::vector< std::thread > curr_tracker;

        for (int gg = 0; gg < multi_req; gg++){

            if (scrape_flag == 1){
                curr_tracker.push_back(std::thread(scrape, infohash, ip, port, std::ref(t_stat)));
            } else if (announce_flag == 1){
                curr_tracker.push_back(std::thread(announce, infohash, ip, port));
            }
            
        }

        t.push_back(std::move(curr_tracker));
        
        if (scrape_flag == 1 && force_flag != 1 ){
            break;
        }
    }

    for (int x = 0; x < t.size(); x++){
        for (int gg = 0; gg < multi_req; gg++){
            t[x][gg].join();
        }
    }

    if (scrape_flag == 1){
        std::cout << "Seeds: " << t_stat.seeds << "\nDownloads: " << t_stat.downloaded << "\nLeechs: " << t_stat.leechs << "\nFrom " << t_stat.servers <<" servers.\n";
    }

    return 0;
}