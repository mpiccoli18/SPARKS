#include <string>
#include <chrono> 

#include "../UAV.hpp"
#include "../puf.hpp"
#include "../utils.hpp"
#include "../connectionHelper.hpp"

std::string idA = "A";
std::string idB = "B";
bool server = false;

int enrolment_client(UAV * A){
    std::cout << "\nEnrolment process begins.\n";

    // A enroll with B
    // A computes the challenge for B
    unsigned char xB[PUF_SIZE];
    generate_random_bytes(xB, PUF_SIZE);
    std::cout << "xB : "; print_hex(xB, PUF_SIZE);

    // Creates B in the memory of A and save xB 
    A->addUAV(idB, xB);

    // Creates the challenge for B
    unsigned char CB[PUF_SIZE];
    A->callPUF(xB, CB);
    std::cout << "CB : "; print_hex(CB, PUF_SIZE);

    // Sends CB
    json msg = {{"id", idA}, {"CB", toHexString(CB, PUF_SIZE)}};
    A->socketModule.sendMessage(msg);
    std::cout << "Sent CB.\n";

    // Wait for B's response (with RB)
    json rsp = A->socketModule.receiveMessage();
    printJSON(rsp);

    unsigned char RB[PUF_SIZE];
    fromHexString(rsp["RB"].get<std::string>(), RB, PUF_SIZE);

    A->getUAVData(idB)->setR(RB);

    std::cout << "\nB is enroled to A\n";

    // B enroll with A
    // A receive CA. It saves CA.
    rsp = A->socketModule.receiveMessage();
    printJSON(rsp);
    unsigned char CA[PUF_SIZE];
    fromHexString(rsp["CA"].get<std::string>(), CA, PUF_SIZE);
    A->getUAVData(idB)->setC(CA);

    // A computes RA
    unsigned char RA[PUF_SIZE];
    A->callPUF(CA, RA);
    std::cout << "RA : "; print_hex(RA, PUF_SIZE);

    // A sends RA
    msg = {{"id", idA}, {"RA", toHexString(RA, PUF_SIZE)}};
    A->socketModule.sendMessage(msg);
    std::cout << "Sent RA.\n";

    return 0;
}

int main(){

    // Creation of the UAV

    UAV A = UAV(idA);

    std::cout << "The client drone id is : " <<A.getId() << ".\n"; 

    A.socketModule.initiateConnection("127.0.0.1", 8080);

    // When the programm reaches this point, the UAV are connected

    int ret = enrolment_client(&A);
    if (ret == 1){
        return ret;
    }

    

    return 0;
}