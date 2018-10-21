#include "../header/Server.h"

size_t Server::checkItem(std::string lock) {
    size_t userID = 0;
    std::map<std::string, size_t>::iterator it;
    it = lockMap.find(lock);
    if(it != lockMap.end()){
        userID = it->second;
    }
    return userID;
};

/*
void Server::addItem(lockpackage lockbag) {
    lockMap[lockbag.lock] = lockbag.user;
};
*/

void Server::deleteItem(std::string lock) {
    lockMap.erase(lock);
};

void Server::updateItem(lockpackage lockbag) {
    lockMap[lockbag.lock] = lockbag.user;
};

bool Server::connectNode(std::string address, std::string port, lockpackage lockbag) {
    int sockfd, numbytes_user, numbytes_lock;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if((rv = getaddrinfo(address.c_str(), port.c_str(), &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return false;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return false;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

    if((numbytes_user = send(sockfd, &lockbag.user, sizeof(lockbag.user), 0) == -1) || 
       (numbytes_lock = send(sockfd, &lockbag.lock, sizeof(lockbag.lock), 0) == -1)) {
           perror("send");
           return false;
    }

	close(sockfd);

	return true;
};