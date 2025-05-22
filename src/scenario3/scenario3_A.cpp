#include <string>
#include <chrono> 
#include <thread>

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

    UAV A = UAV(idA);

    std::cout << "The client drone id is : " <<A.getId() << ".\n"; 

    A.socketModule.initiateConnection(ip, 8080);

    // When the programm reaches this point, the UAV are connected

    int ret = A.enrolment_client();
    if (ret != 0){
        return ret;
    }

    // This one should partially fail bafore the end, triggering the 
    // usage of CAOld in the next auth.
    ret = A.failed_autentication_client();
    std::cout << "The autentication failed because of a timeout.\n";

    // Expected output is a fail
    // A will just retry an autentication later (and it should work)
    
    A.socketModule.closeConnection();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    A.socketModule.initiateConnection(ip,8080);

    ret = A.autentication_client();
    if (ret != 0){
        return ret;
    }    

    A.socketModule.closeConnection();

    return 0;
}