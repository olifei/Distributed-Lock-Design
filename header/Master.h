#ifndef MASTER_H
#define MASTER_H

#include "Server.h"
#include <thread>
#include <functional>

class Master : public Server {
private:
    //std::map<std::string, size_t> lockMap;
    std::list<std::string> slaveList; // Key: ID, value: IP
    std::list<std::string> loadslaveList();
    void saveslaveList(std::list<std::string>);
    bool daemon(); // Service for client and slaves
    void flushSlave(lockpackage); // Flush up-to-date info to slaves
    
public:
    Master(std::string); // Own IP
};

#endif