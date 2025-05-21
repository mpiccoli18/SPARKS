#include <string>
#include <chrono> 
#include <thread>

#include "../UAV.hpp"
#include "../puf.hpp"
#include "../utils.hpp"
#include "../SocketModule.hpp"

std::string idA = "A";
std::string idC = "C";
std::string idBS = "BS";

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Error: No IPs addresses provided. Please provide the IPs as arguments." << std::endl;
        return 1;  // Exit with an error code
    }

    const char* ipBS = argv[1];  // Read IP from command-line argument
    const char* ipUAV = argv[2];  // Read IP from command-line argument

    std::cout << "Using IP BS: " << ipBS << std::endl;
    std::cout << "Using IP UAV: " << ipUAV << std::endl;

    // Creation of the UAV

    UAV C = UAV(idC);

    std::cout << "The supplementary drone id is : " << C.getId() << ".\n"; 

    // Connect to the BS to retrieve A's credentials
    C.socketModule.initiateConnection(ipBS, 8080);

    // A's credential retrieval
    int ret = C.preEnrolmentRetrival();
    if (ret == 1){
        return ret;
    }

    C.socketModule.closeConnection();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // C will now try to connect to A

    C.socketModule.initiateConnection(ipUAV,8085);

    ret = C.supplementaryAuthenticationSup();
    if (ret == 1){
        return ret;
    }

    return 0;
}