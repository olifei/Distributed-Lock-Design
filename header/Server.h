#ifndef SERVER_H
#define SERVER_H

#include <map>
#include "Node.h"
#define MAXDATASIZE 1000

class Server {
protected:
    std::string selfIP;
    size_t selfID;
    std::map<std::string, size_t> lockMap;

    bool connectNode(std::string, std::string, lockpackage); // IP, PORT, package
    // bool daemon(std::string); // Listen ports

public:
    Server(){};
    size_t checkItem(std::string); // Lockname
    // void addItem(lockpackage); // UID lockname
    void deleteItem(std::string); // Lockname
    void updateItem(lockpackage); // UID lockname
};

#endif