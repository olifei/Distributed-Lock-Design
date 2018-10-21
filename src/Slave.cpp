#include "../header/Slave.h"

Slave::Slave(std::string self, std::string master) {
    selfIP = self;
    masterIP = master;
    std::hash<std::string> str_hash;
    selfID = str_hash(selfIP);

    if(synchronize() == false) {
        fprintf(stderr, "Error: synchronization to %s failed!\n", masterIP);
    }
    std::thread thread_client(daemon_client, CSPORT);
    thread_client.detach();
    std::thread thread_master(daemon_master, MSPORT);
    thread_master.detach();
};

bool Slave::synchronize() {
    lockpackage synbag;
    synbag.service = 4;
    void* buf = (void*)malloc(sizeof(1000));
    int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(masterIP.c_str(), std::to_string(CSPORT).c_str(), &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return false;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("slave: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("slave: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "slave: failed to connect\n");
		return false;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("slave: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	if (send(sockfd, &synbag, sizeof(synbag), 0) == -1) {
	    perror("send");
	    return false;
	}

    if (recv(sockfd, &buf, 1000, 0) == -1) {
	    perror("recv");
	    return false;
	}
	
    std::map<std::string, size_t>* recvmap = (std::map<std::string, size_t>*)buf;
    lockMap = *recvmap;

	close(sockfd);

	return true;
};

bool Slave::reportMaster(lockpackage lockbag) {
    return connectNode(masterIP, std::to_string(MSPORT), lockbag);
}

bool Slave::daemon_client() {
    int sockfd, new_fd; // Listen on socke_fd, new connection on new_fd
    void* buf = (void*)malloc(sizeof(1000));
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

		if(!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener
			if (recv(new_fd, buf, 1000, 0) == -1)
				perror("recv");
            lockpackage* recvbag = (lockpackage*)buf;
            unsigned short recvservice = recvbag->service;
            switch(recvservice) {
                case 1: // check
                    size_t currentuser = checkItem(recvbag->lock);
                    if(send(new_fd, &currentuser, sizeof(currentuser), 0) == -1)
                        perror("send_reply_check");
                    break;

                case 2: // update
                    reportMaster(*recvbag);
                    break;

                case 3: // delete
                    reportMaster(*recvbag);
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

bool Slave::daemon_master() {
    int sockfd, new_fd; // Listen on socke_fd, new connection on new_fd
    void* buf = (void*)malloc(sizeof(1000));
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

    if((rv = getaddrinfo(NULL, std::to_string(MSPORT).c_str(), &hints, &servinfo)) != 0) {
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

		if(!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener
			if (recv(new_fd, buf, 1000, 0) == -1)
				perror("recv");
            lockpackage* recvbag = (lockpackage*)buf;
            unsigned short recvservice = recvbag->service;
            switch(recvservice) {
                case 2: // update
                    updateItem(*recvbag);
                    break;

                case 3: // delete
                    deleteItem(recvbag->lock);
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