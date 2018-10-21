#include "Server.h"
#include<thread>
#include <functional>

class Master : public Server {
private:
    std::map<std::string, std::string> slaveMap; // Key: ID, value: IP
    bool daemon(); // Service for client and slaves
    void flushSlave(lockpackage); // Flush up-to-date info to slaves
public:
    Master(std::string); // Own IP
};