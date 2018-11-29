#include "../header/Client.h"

Client::Client(std::string self, std::string server) {
    selfIP = self;
    serverIP = server;
    std::hash<std::string> str_hash;
    selfID = str_hash(selfIP);
	std::cout << "hash is " << selfID;
}

bool Client::checkLock(std::string lock) {
    lockpackage lockbag;
    lockbag.user = selfID;
    lockbag.lock = lock;
    lockbag.service = 1;

    int sockfd, numbytes_user, numbytes_lock;
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if((rv = getaddrinfo(serverIP.c_str(), std::to_string(CSPORT).c_str(), &hints, &servinfo)) != 0) {
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
    if (recv(sockfd, buf, 1000, 0) == -1)
        perror("recv");
	
    printf("The owner of the lock %s is %d.\n", lock.c_str(), *(size_t*)buf);
	close(sockfd);
    return true;
}

void Client::updateItem(unsigned short choice, std::string lock) {
    lockpackage lockbag;
    lockbag.user = selfID;
    lockbag.lock = lock;
    lockbag.service = choice;

    int sockfd, numbytes_user, numbytes_lock;
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if((rv = getaddrinfo(serverIP.c_str(), std::to_string(CSPORT).c_str(), &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
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
    }
	close(sockfd);
}

int main (int argc, char *argv[]) {
    if(argc != 3)
        return 1;
    Client client(argv[1], argv[2]);
    unsigned short service;
    std::string lock;
    while(1) {
        std::cout << "Choose a service: check 1, update 2, delete 3" << std::endl;
        std::cin >> service;
        switch(service) {
            case 1:
                std::cout << "Now enter the lockname you would like to check: ";
                std::cin >> lock;
                client.checkLock(lock);
                break;

            case 2:
                std::cout << "Now enter the lockname you would like to update: ";
                std::cin >> lock;
                client.updateItem(service, lock);
                break;
            
            case 3:
                std::cout << "Now enter the lockname you would like to delete: ";
                std::cin >> lock;
                client.updateItem(service, lock);
                break;
            
            default:
                std::cout << "Error: Invalid service!" << std::endl;
        }
    }
    return 0;
}
