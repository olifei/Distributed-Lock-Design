#include "../header/Master.h"

Master::Master(std::string self) {
    selfIP = self;
    std::hash<std::string> str_hash;
    selfID = str_hash(selfIP);
    serializeMap(lockMap);
    daemon();
};

bool Master::daemon() {
    int sockfd, new_fd; // Listen on socke_fd, new connection on new_fd
    void* buf = (void*)malloc(1000);
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // Connector's address info
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

    if((rv = getaddrinfo(NULL, std::to_string(CSPORT).c_str(), &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return false;
	}

    // loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			return false;
		}

		if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

    freeaddrinfo(servinfo); // all done with this structure

    if(p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		return false;
	}

	if(listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		return false;
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		return false;
	}

    printf("server: waiting for connections...\n");

    while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if(new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);
        std::string slaveip(s);

        lockMap = deserializeMap();
        size_t currentuser;
        std::ostringstream ssmap;
        boost::archive::text_oarchive oarch(ssmap);
        oarch << lockMap;
        const std::string sendmess = ssmap.str();
        const char* sendmess_char = sendmess.c_str();

		if(!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener


			if (recv(new_fd, buf, 1000, 0) == -1)
				perror("recv");
            std::istringstream ss((char*)buf);
			boost::archive::text_iarchive iarch(ss);
			lockpackage temp;
			iarch >> temp;
            lockpackage* recvbag = &temp;
            
            unsigned short recvservice = recvbag->service;

            switch(recvservice) {
                case 1: // check
                    {
                        currentuser = checkItem(recvbag->lock);
                        if(send(new_fd, &currentuser, sizeof(size_t), 0) == -1)
                            perror("send_reply_check");
                    }
                    break;

                case 2: // update

                    currentuser = checkItem(recvbag->lock);
                    
                    std::cout << "Current user is " << currentuser << std::endl;

                    if(currentuser == 0 || currentuser == recvbag->user) { // no result in current map
                        updateItem(*recvbag);                            // or currentuser own the lock
                        flushSlave(*recvbag);
                    }
                    else
                        fprintf(stderr, "Lock %s is owned by user %d\n",
                                recvbag->lock.c_str(), currentuser);

                    break;

                case 3: // delete
                    currentuser = checkItem(recvbag->lock);
                    if(currentuser == 0)
                        fprintf(stderr, "No such lock %s\n", recvbag->lock.c_str());

                    else if(currentuser != recvbag->user) 
                        fprintf(stderr, "User %d can't modify lock %s, which is owned by %d\n",
                                recvbag->user, recvbag->lock.c_str(), currentuser);
                    
                    else {
                        deleteItem(recvbag->lock);
                        flushSlave(*recvbag);
                    }
                    break;

                case 4: // synchronized
                    slaveList.assign(1, slaveip);
                    saveslaveList(slaveList);
                    if(send(new_fd, sendmess_char, strlen(sendmess_char), 0) == -1)
                        perror("synchronization");
                    break;
                    
                default: // error
                    fprintf(stderr, "invalide service tag\n");
            }
			close(new_fd);
			return true;
		}
		close(new_fd);  // parent doesn't need this

	}
    free(buf);
	return true;
}

void Master::flushSlave(lockpackage lockbag) {
    //std::cout << "broadcast map" << std::endl;
    //connectNode("192.168.0.38", std::to_string(MSPORT), lockbag);
    slaveList = loadslaveList();
    for(auto& x: slaveList) {
        std::cout << "flushing map to " << x << std::endl;
        connectNode(x, std::to_string(MSPORT), lockbag);
    }
}

std::list<std::string> Master::loadslaveList() {
    std::list<std::string> tmp;
	std::ifstream listFile("slavelist.txt");
	boost::archive::text_iarchive ia(listFile);
	ia >> tmp;
	listFile.close();
	std::cout << "loading List..." << std::endl;
	return tmp;
}

void Master::saveslaveList(std::list<std::string> tmp) {
    std::ofstream listFile("slavelist.txt");
	boost::archive::text_oarchive oa(listFile);
	oa << tmp;
	listFile.close();
	std::cout << "saving List..." << std::endl;;
}

int main (int argc, char *argv[]) {
    if(argc != 2)
        return 1;
    Master master(argv[1]);
    return 0;
}