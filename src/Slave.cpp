#include "../header/Slave.h"
#include <functional>

Slave::Slave(std::string self, std::string master) {
    selfIP = self;
    masterIP = master;
    std::hash<std::string> str_hash;
    selfID = str_hash(selfIP);

    daemon(CPORT_RX);
    daemon(MPROT_RX);
    /*
    if(connectNode(masterIP) == false) {
        fprintf(stderr, "Error: connection to %s failed!\n", masterIP);
        exit(EXIT_FAILURE);
    }
    */
    if(synchronize(masterIP) == false) {
        fprintf(stderr, "Error: synchronization to %s failed!\n", masterIP);
    }
};

bool Slave::synchronize(std::string master) {
    // TODO
};

bool Slave::reportMaster(std::string master, size_t user, std::string lock) {
    return connectNode(master, user, lock);
}

