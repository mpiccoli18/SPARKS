#include <string>
#include <chrono> 

#include "../UAV.hpp"
#include "../puf.hpp"
#include "../utils.hpp"
#include "../SocketModule.hpp"

std::string idA = "A";
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Error: No IP address provided. Please provide the IP as an argument." << std::endl;
        return 1;  // Exit with an error code
    }
    UAV A(idA);
    std::unordered_map<std::string, std::string> msg;
    std::cout << "The client drone id is : " << A.getId() << ".\n";
    const char* ip = argv[1];  // Read IP from command-line argument
    std::cout << "Using IP: " << ip << std::endl;
    A.socketModule.initiateConnection(ip, 8080);
    A.socketModule.receiveMsg(msg);
    PROD_ONLY({printMsg(msg);});
    A.enrolment_client();  // Start the enrolment process
    //A.startListeningThread();  // Start the listening thread
    //A.getThread()->join();  // Wait for the thread to finish
    return 0;
}