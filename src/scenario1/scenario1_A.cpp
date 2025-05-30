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

    int ret = A.enrolment_client();
    if (ret == 1){
        return ret;
    }

    ret = A.autentication_client();
    if (ret == 1){
        return ret;
    }

    A.socketModule.closeConnection();

    return 0;
}
