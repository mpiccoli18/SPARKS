#include <string>
#include <chrono> 

#include "../UAV.hpp"
#include "../puf.hpp"
#include "../utils.hpp"
#include "../connectionHelper.hpp"

std::string idA = "A";
std::string idB = "B";
bool server = true;

int enrolment_server(UAV * B){
    std::cout << "\nEnrolment process begins.\n";

    // B waits for B's message  (with CB)
    json rsp = B->socketModule.receiveMessage();
    printJSON(rsp);

    // B receive CB. It creates A in the memory of B and save CB.
    unsigned char CB[PUF_SIZE];
    fromHexString(rsp["CB"].get<std::string>(), CB, PUF_SIZE);
    B->addUAV(idA, nullptr, CB);

    // B computes RB
    unsigned char RB[PUF_SIZE];
    B->callPUF(CB, RB);
    std::cout << "RB : "; print_hex(RB, PUF_SIZE);

    // B sends RB
    json msg = {{"id", idB}, {"RB", toHexString(RB, PUF_SIZE)}};
    B->socketModule.sendMessage(msg);
    std::cout << "Sent RB.\n";

    // B enroll with A
    // Computes xA
    unsigned char xA[PUF_SIZE];
    generate_random_bytes(xA, PUF_SIZE);
    std::cout << "xA : "; print_hex(xA, PUF_SIZE);

    // Save xA
    B->getUAVData(idA)->setX(xA);

    // Creates the challenge for A
    unsigned char CA[PUF_SIZE];
    B->callPUF(xA, CA);
    std::cout << "CA : "; print_hex(CA, PUF_SIZE);

    // Sends CA
    msg = {{"id", idB}, {"CA", toHexString(CA, PUF_SIZE)}};
    B->socketModule.sendMessage(msg);
    std::cout << "Sent CA.\n";

    // B receive RA and saves it. 
    rsp = B->socketModule.receiveMessage();
    printJSON(rsp);
    unsigned char RA[PUF_SIZE];
    fromHexString(rsp["RA"].get<std::string>(), RA, PUF_SIZE);
    B->getUAVData(idA)->setR(RA);
    
    return 0;
}

int main(){

    // Creation of the UAV

    UAV B = UAV(idB);

    std::cout << "The server drone id is : " <<B.getId() << ".\n"; 

    B.socketModule.waitForConnection(8080);

    // When the programm reaches this point, the UAV are connected

    int ret = enrolment_server(&B);
    if (ret == 1){
        return ret;
    }

    

    return 0;
}