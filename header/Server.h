#include <map>
#include <string>
#include "Node.h"

class Server : public Node <std::map<std::string, int>, std::string, std::string> {
protected:
    std::string selfIP;
    size_t selfID;
    std::map<std::string, size_t> lockMap;

    bool connectNode(std::string, std::size_t, std::string); // send (ID, lockname)
    void daemon(size_t); // lesson ports

public:
    Server();
    size_t checkItem(std::string); // Lockname
    void addItem(size_t, std::string); // UID lockname
    void deleteItem(std::string); // lockname
    void updateItem(size_t, std::string); // UID lockname
};