#include <string>
#include <chrono> 

#include "../UAV.hpp"
#include "../puf.hpp"
#include "../utils.hpp"
#include "../SocketModule.hpp"

std::string idB = "B";
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Error: No IP address provided. Please provide the IP as an argument." << std::endl;
        return 1;  // Exit with an error code
    }
    
    UAV B(idB);
    std::cout << "The client drone id is : " << B.getId() << ".\n";    
    const char* ip = argv[1];  // Read IP from command-line argument
    std::cout << "Using IP: " << ip << std::endl;
    //B.socketModule.waitForConnection(8080);  // Wait for connection from A
    B.startListeningThread();  // Start the listening thread
    B.getThread()->join();  // Wait for the thread to finish
    
    return 0;
}