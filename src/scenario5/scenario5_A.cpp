#include <string>
#include <chrono> 

#include "../UAV.hpp"
#include "../puf.hpp"
#include "../utils.hpp"
#include "../SocketModule.hpp"

std::string idA = "A";
std::string idB = "B";
bool server = false;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Error: No IP address provided. Please provide the IP as an argument." << std::endl;
        return 1;  // Exit with an error code
    }

    const char* ip = argv[1];  // Read IP from command-line argument

    std::cout << "Using IP: " << ip << std::endl;

    // Creation of the UAV

    UAV A(idA);

    std::cout << "The client drone id is : " <<A.getId() << ".\n"; 

    A.socketModule.initiateConnection(ip, 8080);

    // When the programm reaches this point, the UAV are connected

    unsigned char M0[PUF_SIZE];
    generate_random_bytes(M0,PUF_SIZE);
    unsigned char M1[PUF_SIZE];
    generate_random_bytes(M1,PUF_SIZE);

    A.socketModule.createMap(2);
    A.socketModule.addKeyValue("id", idA);
    A.socketModule.addKeyValue("M0", M0, PUF_SIZE);

    A.socketModule.sendMsg();

    A.socketModule.createMap(2);
    A.socketModule.addKeyValue("id", idA);
    A.socketModule.addKeyValue("M1", M1, PUF_SIZE);

    A.socketModule.sendMsg();

    A.socketModule.closeConnection();

    return 0;
}
