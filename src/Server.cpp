#include "../header/Server.h"

bool Server::connectNode(std::string address, size_t user, std::string lock) {
    // TODO
};

void Server::daemon(size_t port) {
    // TODO
}

size_t Server::checkItem(std::string lock) {
    size_t userID = 0;
    std::map<std::string, size_t>::iterator it;
    it = lockMap.find(lock);
    if(it != lockMap.end()){
        userID = it->second;
    }
    return userID;
};


void Server::addItem(size_t userID, std::string lock) {
    lockMap[lock] = userID;
};

void Server::deleteItem(std::string lock) {
    lockMap.erase(lock);
};

void Server::updateItem(size_t userID, std::string lock) {
    lockMap[lock] = userID;
};