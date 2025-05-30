#include <string>
#include <chrono> 

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
    if (ret == 1){
        return ret;
    }

    ret = B.autentication_server();
    if (ret == 1){
        return ret;
    }
    
    B.socketModule.closeConnection();

    return 0;
}
