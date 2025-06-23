#include <string>
#include <chrono> 

#include "../UAV.hpp"
#include "../puf.hpp"
#include "../utils.hpp"
#include "../SocketModule.hpp"

std::string idA = "A";
std::string idC = "C";
std::string idBS = "BS";

int main(){

    // Creation of the base station

    puf BSpuf;
    unsigned char Lx[CHALLENGE_SIZE][PUF_SIZE];
    unsigned char LR[CHALLENGE_SIZE][PUF_SIZE];
    SocketModule sm;

    // Wait for a connection
    sm.waitForConnection(8080);

    // Someone connected

    // BS generates a list of random numbers as challenges
    for (int i = 0; i < CHALLENGE_SIZE; i++){
        generate_random_bytes(Lx[i], PUF_SIZE);
        std::cout << "Lx[" << i << "]: "; print_hex(Lx[i], PUF_SIZE);
    }
    
    // BS sends those numbers to the PUF to create a list of challenges
    // In a message that will be sent to the UAV to get a list of responses
    unsigned char LC[CHALLENGE_SIZE][PUF_SIZE];
    for (int i = 0; i < CHALLENGE_SIZE; i++){
        BSpuf.process(Lx[i], PUF_SIZE, LC[i]);
        std::cout << "LC[" << i << "]: "; print_hex(LC[i], PUF_SIZE);
    }

    std::cout << "Sent LC to the PUF.\n";

    // This list is sent to the UAV to get the responses
    sm.createMap(2);
    sm.addKeyValue("id", idBS);
    sm.addKeyValue("data", (const unsigned char *)LC, CHALLENGE_SIZE*PUF_SIZE);
    
    sm.sendMsg();
    std::cout << "Sent LC to A;\n";
        
    // Wait for the responses
    std::unordered_map<std::string, std::string> msg;
    msg.reserve(3);
    sm.receiveMsg(msg);

    // Check if an error occurred
    if (msg.empty()) {
        std::cerr << "Error occurred: content is empty!" << std::endl;
    }

    extractValueFromMap(msg,"data",LR[0],CHALLENGE_SIZE*PUF_SIZE);

    // Pre-enrolment done. Close connection.
    sm.closeConnection();

    std::cout << "\nPre-enrolment is done\n";

    // Wait for C to connect
    sm.waitForConnection(8080);

    // C wants A's credentials
    // BS retrieve one challenge and one corresponding answer
    std::cout << "\nC want A's credentials.\n";
    unsigned char xA[PUF_SIZE];
    memcpy(xA, Lx[0], PUF_SIZE);
    std::cout << "xA : "; print_hex(xA, PUF_SIZE);
    unsigned char CA[PUF_SIZE];
    BSpuf.process(xA, PUF_SIZE, CA);
    std::cout << "CA : "; print_hex(CA, PUF_SIZE);
    unsigned char RA[PUF_SIZE];
    memcpy(RA, LR[0],PUF_SIZE);
    std::cout << "RA : "; print_hex(RA, PUF_SIZE);

    // BS sends CA and RA to C

    sm.createMap(3);
    sm.addKeyValue("id", idBS);
    sm.addKeyValue("CA", CA, PUF_SIZE);
    sm.addKeyValue("RA", RA, PUF_SIZE);
    sm.sendMsg();

    msg.clear();

    sm.closeConnection();
    std::cout << "\nGave to C A's credentials.\n";
    std::cout << "\nEnd for Base Station\n";

    return 0;
}