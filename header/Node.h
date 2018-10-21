#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define BACKLOG 10	 // How many pending connections queue will hold

template <typename Map, typename Address, typename Lock> class Node { // Node ADT
private:
    Map map; // Distributed lock map
    Address selfIP; // IP address
    size_t ID; // Node ID

public:
    Node() {}; // Init by its own IP
    virtual ~Node() {};
    virtual bool connect2other(Address) = 0; //Connect to other node
    virtual size_t checkItem(Lock) = 0; // Check map with given lockname
    virtual void addItem(size_t, Lock) = 0; // Add item (UID, lockname) to map
    virtual void deleteItem(Lock) = 0; // Delete item according to lockname
    virtual void updateItem(size_t, Lock) = 0; // Update map
};

/*  service[1] = check
    service[2] = update
    service[3] = delete
    service[4] = syschronize
*/
struct lockpackage{ // data structure of RX/TX package
    unsigned short service;
    size_t user;
    std::string lock;
};
typedef struct lockpackage lockpackage;

// Socker part
void sigchld_handler(int s) {
    (void)s; // quiet unused variable warning
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;
	while(waitpid(-1, NULL, WNOHANG) > 0);
	errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}