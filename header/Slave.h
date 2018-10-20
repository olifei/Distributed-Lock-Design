#include "Server.h"
#define CPORT_RX 15000
#define CPORT_TX 15001
#define MPROT_RX 15002
#define MPROT_TX 15003

class Slave : public Server {
private:
    std::string masterIP;
    bool synchronize(std::string); // Master's IP

public:
    Slave(std::string, std::string); // Own IP, master IP
    bool reportMaster(std::string, size_t, std::string); // report a new update
};