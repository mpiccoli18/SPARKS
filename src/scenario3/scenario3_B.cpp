#include <string>
#include <chrono> 
#include <thread>

#include "../UAV.hpp"
#include "../puf.hpp"
#include "../utils.hpp"
#include "../SocketModule.hpp"

std::string idA = "A";
std::string idB = "B";
bool server = true;

int main(){

    // Creation of the UAV

    UAV B(idB);

    std::cout << "The server drone id is : " <<B.getId() << ".\n"; 

    B.socketModule.waitForConnection(8080);

    // When the programm reaches this point, the UAV are connected

    int ret = B.enrolment_server();
    if (ret != 0){
        return ret;
    }

    ret = B.autentication_server();
    std::cout << "The autentication failed because of a timeout.\n";

    // Expected output is a fail
    // The client timed out. The next autentication should still work tho.
    
    B.socketModule.closeConnection();
    B.socketModule.waitForConnection(8080);

    ret = B.autentication_server();
    if (ret != 0){
        return ret;
    }   
    
    B.socketModule.closeConnection();

    return 0;
}