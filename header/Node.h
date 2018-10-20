template <typename Map, typename Address, typename Lock> class Node { // Node ADT
private:
    Map map; // Distributed lock map
    Address selfIP; // IP address
    size_t ID; // Node ID

public:
    Node() {}; // Init by its own IP
    virtual ~Node() {};
    virtual bool connect(Address) = 0; //Connect to other node
    virtual size_t checkItem(Lock) = 0; // Check map with given lockname
    virtual void addItem(size_t, Lock) = 0; // Add item (UID, lockname) to map
    virtual void deleteItem(Lock) = 0; // delete item according to lockname
    virtual void updateItem(size_t, Lock) = 0; // update map
};