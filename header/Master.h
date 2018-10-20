#include "Server.h"
#define CPORT_RX 15000
#define CPORT_TX 15001
#define SPROT_RX 15002
#define SPROT_TX 15003

class Master : public Server {
private:
    std::map<std::string, std::string> slaveMap; // key: ID, value: IP

public:
    Master(std::string); // Own IP
    void flushSlave(size_t, std::string); // Flush up-to-date info to slaves
};