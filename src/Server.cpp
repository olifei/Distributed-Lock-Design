#include "../header/Server.h"

size_t Server::checkItem(std::string lock) {
	std::cout << "Checking the lock. ";
	std::cout << "Require lock is " << lock << std::endl;
	lockMap = deserializeMap(); //load lockMap
	std::cout << lockMap[lock] << std::endl;
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
	lockMap = deserializeMap(); //load lockMap
    lockMap.erase(lock);
	serializeMap(lockMap); // save lockMap
};

void Server::updateItem(lockpackage lockbag) {
	lockMap = deserializeMap(); //load lockMap
    lockMap[lockbag.lock] = lockbag.user;
	std::cout << "From update service!!!" << std::endl;
	std::cout << "Owner of lock(" << lockbag.lock << ") is " << lockbag.user << std::endl;
	serializeMap(lockMap); // save lockMap
};

bool Server::connectNode(std::string address, std::string port, lockpackage lockbag) {
    int sockfd, numbytes_user, numbytes_lock;
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

	std::ostringstream os;
	boost::archive::text_oarchive oa(os);
	oa << lockbag;
	const std::string content = os.str();
	const char* content_char = content.c_str();

    if(numbytes_user = send(sockfd, content_char, strlen(content_char)+1, 0) == -1) {
           perror("send");
           return false;
    }

	close(sockfd);

	return true;
};

void Server::serializeMap(std::map<std::string, size_t> tmp) {
	std::ofstream mapFile("mapfile.txt");
	boost::archive::text_oarchive oa(mapFile);
	oa << tmp;
	mapFile.close();
	std::cout << "saving Map..." << std::endl;
};

std::map<std::string, size_t> Server::deserializeMap() {
	std::map<std::string, size_t> tmp;
	std::ifstream mapFile("mapfile.txt");
	boost::archive::text_iarchive ia(mapFile);
	ia >> tmp;
	mapFile.close();
	std::cout << "loading Map..." << std::endl;
	return tmp;
}