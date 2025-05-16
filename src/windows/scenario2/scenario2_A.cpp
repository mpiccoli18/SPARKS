#include <string>
#include <chrono> 

#include "../UAV.hpp"
#include "../puf.hpp"
#include "../utils.hpp"
#include "../SocketModule.hpp"

std::string idA = "A";
std::string idBS = "BS";

int preEnrolment(UAV * A){

    // A waits for BS's query
    json rsp = A->socketModule.receiveMessage();
    printJSON(rsp);

    // Check if an error occurred
    if (rsp.contains("error")) {
        std::cerr << "Error occurred: " << rsp["error"] << std::endl;
        return -1;
    }

    if(!rsp.contains("data")){
        std::cerr << "Error occurred: no member data" << std::endl;
        return 1;
    }
    std::vector<std::string> receivedHexList = rsp["data"];

    // Convert each hex string back to unsigned char arrays
    unsigned char receivedArrays[CHALLENGE_SIZE][PUF_SIZE];

    for (size_t i = 0; i < receivedHexList.size() && i < receivedHexList.size(); i++) {
        fromHexString(receivedHexList[i], receivedArrays[i], PUF_SIZE);
    }

    // Generates the responses
    unsigned char LR[CHALLENGE_SIZE][PUF_SIZE];
    for (int i = 0; i < CHALLENGE_SIZE && i < CHALLENGE_SIZE; i++) {
        A->callPUF(receivedArrays[i], LR[i]);
        std::cout << "LR[" << i << "]: "; print_hex(LR[i], PUF_SIZE);
    }

    // Send the responses back
    json msg = {{"id", idA}};
    json dataArray = json::array();

    for (int i = 0; i < CHALLENGE_SIZE; i++) {
        dataArray.push_back(toHexString(LR[i], 32));
    }   
    msg["data"] = dataArray;
    A->socketModule.sendMessage(msg);

    return 0;
}

int supplementaryAuthenticationInitial(UAV * A){

    // Waits for C demands
    json rsp = A->socketModule.receiveMessage();
    printJSON(rsp);

    // Check if an error occurred
    if (rsp.contains("error")) {
        std::cerr << "Error occurred: " << rsp["error"] << std::endl;
        return -1;
    }

    // A retrieve the id of the UAV trying to connect
    std::string idC = rsp["id"].get<std::string>();
    
    //  and search it's table for a corresponding UAV 
    UAVData* data = A->getUAVData(idC);
    if (data) {
        std::cout << "UAVData found! Not supposed to happen ?! Quit.\n" << std::endl;
        return 1;
    } 
    std::cout << "UAVData not found for idC.\n" << std::endl;
    std::cout << "Initiate supplementary authentication\n" << std::endl;
    
    // A generates an nonce NA
    unsigned char NA[PUF_SIZE];
    generate_random_bytes(NA);
    std::cout << "NA : "; print_hex(NA, PUF_SIZE);

    json msg = {{"id", idA}, {"NA", toHexString(NA, PUF_SIZE)}};

    // A sends 
    A->socketModule.sendMessage(msg);
    std::cout << "Sent ID and NA.\n";

    // Waits for C's response 
    rsp = A->socketModule.receiveMessage();
    printJSON(rsp);

    // Check if an error occurred
    if (rsp.contains("error")) {
        std::cerr << "Error occurred: " << rsp["error"] << std::endl;
        return -1;
    }

    // A recover M1, CA and the hash
    unsigned char CA[PUF_SIZE];
    if(!rsp.contains("CA")){
        std::cerr << "Error occurred: no member CA" << std::endl;
        return 1;
    }
    fromHexString(rsp["CA"].get<std::string>(), CA, PUF_SIZE);    
    unsigned char M1[PUF_SIZE];
    if(!rsp.contains("M1")){
        std::cerr << "Error occurred: no member M1" << std::endl;
        return 1;
    }
    fromHexString(rsp["M1"].get<std::string>(), M1, PUF_SIZE);
    unsigned char hash1[PUF_SIZE];
    if(!rsp.contains("hash1")){
        std::cerr << "Error occurred: no member hash1" << std::endl;
        return 1;
    }
    fromHexString(rsp["hash1"].get<std::string>(), hash1, PUF_SIZE);

    // A computes RA using transmitted CA
    unsigned char RA[PUF_SIZE];
    A->callPUF(CA, RA);
    std::cout << "RA : "; print_hex(RA, PUF_SIZE);
    
    // A retrieve NC from M1 
    unsigned char NC[PUF_SIZE];
    xor_buffers(M1, RA, PUF_SIZE, NC);
    std::cout << "NC : "; print_hex(NC, PUF_SIZE);

    // A verify the hash
    unsigned char hash1Check[PUF_SIZE];
    EVP_MD_CTX * ctx = initHash();
    addToHash(ctx, idC);
    addToHash(ctx, CA, PUF_SIZE);
    addToHash(ctx, NC, PUF_SIZE);
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash1Check);
    std::cout << "hash1Check : "; print_hex(hash1Check, PUF_SIZE);

    bool res = memcmp(hash1, hash1Check, PUF_SIZE) == 0;
    std::cout << "A verify C's hash : " << res << "\n";

    if(res == 0){
        std::cout << "The autentication failed.\n";
        return res;

    }

    std::cout << "C's hash has been verified. C is autenticated to A.\n";

    // A will now conputes the new challenge and M2
    unsigned char RAp[PUF_SIZE];
    A->callPUF(NC,RAp);
    std::cout << "RAp : "; print_hex(RAp, PUF_SIZE);

    unsigned char M2[PUF_SIZE];
    xor_buffers(NC,RAp,PUF_SIZE,M2);
    std::cout << "M2 : "; print_hex(M2, PUF_SIZE);

    // A sends M2, and a hash of NB, RA, RAp, NA
    unsigned char hash2[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, NC, PUF_SIZE);
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx, RAp, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash2);
    std::cout << "hash2 : "; print_hex(hash2, PUF_SIZE);

    // TODO : send M2, and a hash of NB, RA, RAp, NA
    msg = {
        {"id", idA},
        {"M2", toHexString(M2, PUF_SIZE)},
        {"hash2", toHexString(hash2, PUF_SIZE)}
    };
    A->socketModule.sendMessage(msg);
    std::cout << "Sent ID, M2 and hash2.\n";

    // A waits for C's ACK
    rsp = A->socketModule.receiveMessage();
    printJSON(rsp);

    // Check if an error occurred
    if (rsp.contains("error")) {
        std::cerr << "Error occurred: " << rsp["error"] << std::endl;
        return -1;
    }

    // Check if an error occurred
    if (rsp.contains("error")) {
        std::cerr << "Error occurred: " << rsp["error"] << std::endl;

        // A reach a timeout or didn't received the ACK 
        std::cout << "Received an empty JSON message!" << std::endl;
    
        // A will save concealed current CA in CAOld
        unsigned char xLock[PUF_SIZE];
        generate_random_bytes(xLock);
    
        unsigned char lock[PUF_SIZE];
        A->callPUF(xLock, lock);
    
        unsigned char concealedCA[PUF_SIZE];
        xor_buffers(CA,lock,PUF_SIZE,concealedCA);
    
        A->getUAVData(idC)->setXLock(xLock);
        A->getUAVData(idC)->setSecret(concealedCA);

        // Then A saves the new challenge in CA
        A->addUAV(idC, nullptr, NC);

        return 1;
    }

    // Then A saves the new challenge in CA
    A->addUAV(idC, nullptr, NC);

    // Finished
    std::cout << "\nThe two UAV autenticated each other.\n";

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Error: No IP address provided. Please provide the IP as an argument." << std::endl;
        return 1;  // Exit with an error code
    }

    const char* ip = argv[1];  // Read IP from command-line argument

    std::cout << "Using IP: " << ip << std::endl;

    // Creation of the UAV

    UAV A = UAV(idA);

    std::cout << "The initial drone id is : " <<A.getId() << ".\n"; 

    // Initiate connection

    A.socketModule.initiateConnection(ip,8080);

    // When the programm reaches this point, the UAV are connected

    int ret = preEnrolment(&A);
    if (ret == 1){
        return ret;
    }

    // Pre-enrolment is done, close the socket
    A.socketModule.closeConnection();

    // Wait for C to connect
    A.socketModule.waitForConnection(8085);

    // C connected
    ret = supplementaryAuthenticationInitial(&A);
    if (ret == 1){
        return ret;
    }    

    return 0;
}