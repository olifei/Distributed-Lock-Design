#ifndef SLAVE_H
#define SLAVE_H

#include "Server.h"
#include <thread>
#include <functional>

class Slave : public Server {
private:
    std::string masterIP;
    bool synchronize(); // Master's IP
    bool reportMaster(lockpackage); // Report a new update

public:
    Slave(std::string, std::string); // Own IP, master IP
    bool daemon_client();
    bool daemon_master();
};

#endif