#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <map>
#include <functional>
#include <iostream>
#include "Node.h"
#define MAXDATASIZE 1000

class Client {
public:
    Client(std::string, std::string); // SelfIP, server IP
    bool checkLock(std::string); // Lock name
    void updateItem(unsigned short, std::string); // Choice, lockname
private:
    std::string selfIP;
    size_t selfID;
    std::string serverIP;
};

#endif