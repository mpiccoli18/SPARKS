#include <string>
#include <chrono> 

#include "../UAV.hpp"
#include "../puf.hpp"
#include "../utils.hpp"
#include "../SocketModule.hpp"

std::string idC = "C";
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Error: No IP address provided. Please provide the IP as an argument." << std::endl;
        return 1;  // Exit with an error code
    }
    UAV C(idC);
    std::unordered_map<std::string, std::string> msg;
    std::cout << "The client drone id is : " << C.getId() << ".\n";
    const char* ip = argv[1];  // Read IP from command-line argument
    std::cout << "Using IP: " << ip << std::endl;
    C.socketModule.initiateConnection(ip, 8080);
    C.socketModule.sendMsg({{"id", C.getId()}});  // Send ID to the server

    C.socketModule.receiveMsg(msg);  // Wait for the server to respond
    PROD_ONLY({printMsg(msg);});
    PROD_ONLY({std::cout << "Waiting for server response...\n";});
    C.enrolment_client();
    C.autentication_client();
    C.autentication_key_client();
    C.socketModule.closeConnection();
    std::cout << "C has successfully established a secure connection with B.\n";

    return 0;
}