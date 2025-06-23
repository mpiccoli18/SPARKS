#include <string>
#include <thread> 
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

    std::unordered_map<std::string, std::string> msg1;
    msg1.reserve(2);
    std::unordered_map<std::string, std::string> msg2;
    msg2.reserve(2);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    B.socketModule.receiveMsg(msg1);

    unsigned char M0[PUF_SIZE];
    extractValueFromMap(msg1,"M0",M0,PUF_SIZE);
    std::cout << "M0 : "; print_hex(M0, PUF_SIZE);

    B.socketModule.receiveMsg(msg2);

    unsigned char M1[PUF_SIZE];
    extractValueFromMap(msg2,"M1",M1,PUF_SIZE);
    std::cout << "M1 : "; print_hex(M1, PUF_SIZE);

    B.socketModule.closeConnection();

    return 0;
}
