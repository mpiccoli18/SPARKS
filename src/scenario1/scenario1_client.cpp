#include <string>
#include <chrono> 

#include "../UAV.hpp"
#include "../puf.hpp"
#include "../utils.hpp"
#include "../SocketModule.hpp"

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

    // Check if an error occurred
    if (rsp.contains("error")) {
        std::cerr << "Error occurred: " << rsp["error"] << std::endl;
        return -1;
    }

    unsigned char RB[PUF_SIZE];
    if(!rsp.contains("RB")){
        std::cerr << "Error occurred: no member RB" << std::endl;
        return 1;
    }
    fromHexString(rsp["RB"].get<std::string>(), RB, PUF_SIZE);

    A->getUAVData(idB)->setR(RB);

    std::cout << "\nB is enroled to A\n";

    // B enroll with A
    // A receive CA. It saves CA.
    rsp = A->socketModule.receiveMessage();
    printJSON(rsp);

    // Check if an error occurred
    if (rsp.contains("error")) {
        std::cerr << "Error occurred: " << rsp["error"] << std::endl;
        return -1;
    }

    unsigned char CA[PUF_SIZE];
    if(!rsp.contains("CA")){
        std::cerr << "Error occurred: no member CA" << std::endl;
        return 1;
    }
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

int autentication_client(UAV * A){
    // The client initiate the authentication process
    std::cout << "\nAutentication process begins.\n";

    // A generates a nonce NA 
    unsigned char NA[PUF_SIZE];
    generate_random_bytes(NA);
    std::cout << "NA : "; print_hex(NA, PUF_SIZE);

    const unsigned char * CA = A->getUAVData(idB)->getC();
    if (CA == nullptr){
        std::cout << "No expected challenge in memory for this UAV.\n";
        return 1;
    }
    
    unsigned char M0[PUF_SIZE];
    xor_buffers(NA,CA,PUF_SIZE,M0);
    std::cout << "M0 : "; print_hex(M0, PUF_SIZE);

    // A sends its ID and NA to B 
    json msg = {{"id", idA}, {"M0", toHexString(M0, PUF_SIZE)}};
    A->socketModule.sendMessage(msg);
    std::cout << "Sent ID and M0.\n";

    // A waits for the answer
    json rsp = A->socketModule.receiveMessage();
    printJSON(rsp);

    // Check if an error occurred
    if (rsp.contains("error")) {
        std::cerr << "Error occurred: " << rsp["error"] << std::endl;
        return -1;
    }

    // A recover M1 and the hash
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

    // A computes RA using CA in memory
    std::cout << "CA : "; print_hex(CA, PUF_SIZE);
    unsigned char RA[PUF_SIZE];
    A->callPUF(CA,RA);
    std::cout << "RA : "; print_hex(RA, PUF_SIZE);
    
    // A retrieve NB from M1 
    unsigned char NB[PUF_SIZE];
    xor_buffers(M1, NA, PUF_SIZE, NB);
    xor_buffers(NB, RA, PUF_SIZE, NB);
    std::cout << "NB : "; print_hex(NB, PUF_SIZE);

    // A verify the hash
    unsigned char hash1Check[PUF_SIZE];
    EVP_MD_CTX * ctx = initHash();
    addToHash(ctx, CA, PUF_SIZE);
    addToHash(ctx, NB, PUF_SIZE);
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash1Check);
    std::cout << "hash1Check : "; print_hex(hash1Check, PUF_SIZE);

    bool res = memcmp(hash1, hash1Check, PUF_SIZE) == 0;
    std::cout << "A verify B's hash : " << res << "\n";

    if(res == 0){
        std::cout << "The autentication failed. A will try to verify the hash with an old challenge if it exists.\n";

        // A will recover the old challenge 
        const unsigned char * xLock = A->getUAVData(idB)->getXLock();
        if (xLock == nullptr){
            std::cout << "No old challenge in memory for the requested UAV.\n";
            return 1;
        }
        const unsigned char * secret = A->getUAVData(idB)->getSecret();
        if (secret == nullptr){
            std::cout << "No old challenge in memory for the requested UAV.\n";
            return 1;
        }
        unsigned char lock[PUF_SIZE];
        A->callPUF(xLock, lock);
        unsigned char CAOld[PUF_SIZE]; 
        xor_buffers(lock, secret, PUF_SIZE, CAOld);

        // A will calculate the Nonce A that the server calculated with the wrong CA 
        unsigned char NAOld[PUF_SIZE];
        xor_buffers(M0, CAOld, PUF_SIZE, NAOld);
        std::cout << "NAOld : "; print_hex(NAOld, PUF_SIZE);

        // A will calculate the old response 
        unsigned char RAOld[PUF_SIZE];
        A->callPUF(CAOld, RAOld);
        std::cout << "RAOld : "; print_hex(RAOld, PUF_SIZE);

        // A will deduce NB from the old response
        unsigned char NBOld[PUF_SIZE];
        xor_buffers(M1, RAOld, PUF_SIZE, NBOld);
        xor_buffers(NBOld, NAOld, PUF_SIZE, NBOld);
        xor_buffers(NBOld, NAOld, PUF_SIZE, NBOld);
        std::cout << "NBOld : "; print_hex(NBOld, PUF_SIZE);

        // A now tries to verify the hash with this value
        ctx = initHash();
        addToHash(ctx, CAOld, PUF_SIZE);
        addToHash(ctx, NBOld, PUF_SIZE);
        addToHash(ctx, RAOld, PUF_SIZE);
        addToHash(ctx, NAOld, PUF_SIZE);
        addToHash(ctx, NAOld, PUF_SIZE);
        calculateHash(ctx, hash1Check);

        res = memcmp(hash1, hash1Check, PUF_SIZE) == 0;
        if (res == 0){
            std::cout << "Even with the old challenge, autentication has failed.\n";
            return 1;
        }
        std::cout << "B has been autenticated by A with the old challenge.\n";

        // A will now change the values to be the one obtained of the old challenge
        A->getUAVData(idB)->setC(CAOld);
        memcpy(RA, RAOld, PUF_SIZE);
        memcpy(NB, NBOld, PUF_SIZE);
        memcpy(NA, NAOld, PUF_SIZE);
        memcpy(NA, NAOld, PUF_SIZE);

    }

    std::cout << "B's hash has been verified. B is autenticated to A.\n";

    // A will now conputes the new challenge and M2
    unsigned char RAp[PUF_SIZE];
    A->callPUF(NB,RAp);
    std::cout << "RAp : "; print_hex(RAp, PUF_SIZE);

    unsigned char M2[PUF_SIZE];
    xor_buffers(NB,RAp,PUF_SIZE,M2);
    std::cout << "M2 : "; print_hex(M2, PUF_SIZE);

    // A sends M2, and a hash of NB, RA, RAp, NA
    unsigned char hash2[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, NB, PUF_SIZE);
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx, RAp, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash2);
    std::cout << "hash2 : "; print_hex(hash2, PUF_SIZE);

    // Send M2, and a hash of NB, RA, RAp, NA
    msg = {
        {"id", idA},
        {"M2", toHexString(M2, PUF_SIZE)},
        {"hash2", toHexString(hash2, PUF_SIZE)}
    };
    A->socketModule.sendMessage(msg);
    std::cout << "Sent ID, M2 and hash2.\n";

    // A waits for B's ACK
    rsp = A->socketModule.receiveMessage();
    printJSON(rsp);

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
    
        A->getUAVData(idB)->setXLock(xLock);
        A->getUAVData(idB)->setSecret(concealedCA);

        // Then A saves the new challenge in CA
        A->getUAVData(idB)->setC(NB);

        return 1;
    }

    // Then A saves the new challenge in CA
    A->getUAVData(idB)->setC(NB);

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

    std::cout << "The client drone id is : " <<A.getId() << ".\n"; 

    A.socketModule.initiateConnection(ip, 8080);

    // When the programm reaches this point, the UAV are connected

    int ret = enrolment_client(&A);
    if (ret == 1){
        return ret;
    }

    ret = autentication_client(&A);
    if (ret == 1){
        return ret;
    }

    A.socketModule.closeConnection();

    return 0;
}
