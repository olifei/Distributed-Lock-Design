# -lboost_serialization
cd ./src

g++ -o ../release/Slave Slave.cpp Node.cpp Server.cpp libboost_serialization.a
g++ -o ../release/Client Client.cpp Node.cpp libboost_serialization.a
g++ -o ../release/Master Master.cpp Node.cpp Server.cpp libboost_serialization.a
