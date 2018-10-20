#include "../header/Master.h"
#include <functional>

Master::Master(std::string self) {
    selfIP = self;
    std::hash<std::string> str_hash;
    selfID = str_hash(selfIP);
    daemon(CPORT_RX);
    daemon(SPROT_RX);
};

void Master::flushSlave(size_t user, std::string lock) {
    for(auto& x: slaveMap) {
        connectNode(x.second, user, lock);
    }
}