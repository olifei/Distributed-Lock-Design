# BigDataProject2
Project 2: Distributed Lock Design
1. **Design a simple consensus system, which satisfy the following requirements**
* Contain one leader server and multiple follower server
* Each follower server has a replicated map, the map is consisted with the leader server
* The key of map is the name of distributed lock, and the value is the Client ID who owns the distributed lock

![image](http://www.cs.sjtu.edu.cn/~wuct/bdpt2/pic/project2_pic1.png)

2. **Support multiple clients to preempt/release a distributed lock, and check the owner of a distributed lock**

* For preempting a distributed lock
- If the lock doesn't exist, preempt success
- Otherwise, preempt fail

* For releasing a distributed lock
- If the client owns the lock, release success
- Otherwise, release fail

* For checking a distributed lock
- Any client can check the owner of a distributed lock

3. To ensure the data consistency of the system, the follower servers send all preempt/release requests to the leader server

4. To check the owner of a distributed lock, the follower server accesses its map directly and sends the results to the clients

5. When the leader server handling preempt/release requests
* If needed, modify its map and sends a request propose to all follower servers

* When a follower server receives a request propose
- modify its local map
- check the request is pending or not
- if the request is pending, send an answer to the client

6. In this system, all clients provide preempt/release/check distributed lock interface

7. When a client is initialized
* Define the IP address of the target server
* Generate the Client ID information based on the user information(UUID)

8. Data structure of a client in the consensus system
![image](http://www.cs.sjtu.edu.cn/~wuct/bdpt2/pic/project2_pic2.png)
