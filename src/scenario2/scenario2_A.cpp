#include <string>
#include <chrono> 

#include "../UAV.hpp"
#include "../puf.hpp"
#include "../utils.hpp"
#include "../SocketModule.hpp"

std::string idA = "A";
std::string idBS = "BS";

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Error: No IP address provided. Please provide the IP as an argument." << std::endl;
        return 1;  // Exit with an error code
    }

    const char* ip = argv[1];  // Read IP from command-line argument

    std::cout << "Using IP: " << ip << std::endl;

    // Creation of the UAV

    UAV A = UAV(idA);

    std::cout << "The initial drone id is : " <<A.getId() << ".\n"; 

    // Initiate connection

    A.socketModule.initiateConnection(ip,8080);

    // When the programm reaches this point, the UAV are connected

    int ret = A.preEnrolment();
    if (ret == 1){
        return ret;
    }

    // Pre-enrolment is done, close the socket
    A.socketModule.closeConnection();

    // Wait for C to connect
    A.socketModule.waitForConnection(8085);

    // C connected
    ret = A.supplementaryAuthenticationInitial();
    if (ret == 1){
        return ret;
    }    

    return 0;
}