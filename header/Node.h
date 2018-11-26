#ifndef NODE_H
#define NODE_H

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
#include <string>
#include <sstream>
#include <iostream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/map.hpp>

#define BACKLOG 10	 // How many pending connections queue will hold
#define MSPORT 15000 // master <---> slave
#define CSPORT 15001 // client <---> server

template <typename Map, typename Address, typename Lock> class Node { // Node ADT
private:
    Map map; // Distributed lock map
    Address selfIP; // IP address
    size_t ID; // Node ID

public:
    Node() {}; // Init by its own IP
    virtual ~Node() {};
    virtual bool connect2other(Address) = 0; //Connect to other node
    virtual size_t checkItem(Lock) = 0; // Check map with given lockname
    virtual void addItem(size_t, Lock) = 0; // Add item (UID, lockname) to map
    virtual void deleteItem(Lock) = 0; // Delete item according to lockname
    virtual void updateItem(size_t, Lock) = 0; // Update map
};

/*  service[1] = check
    service[2] = update
    service[3] = delete
    service[4] = syschronize
*/
struct lockpackage{ // data structure of RX/TX package
    unsigned short service;
    size_t user;
    std::string lock;
    
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version = 1) {
        ar & service;
        ar & user;
        ar & lock;
    }
};
typedef struct lockpackage lockpackage;

void *get_in_addr(struct sockaddr*);
void sigchld_handler(int);

#endif