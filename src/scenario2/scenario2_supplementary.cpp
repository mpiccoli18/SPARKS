#include <string>
#include <chrono> 

#include "../UAV.hpp"
#include "../puf.hpp"
#include "../utils.hpp"
#include "../connectionHelper.hpp"

std::string idA = "A";
std::string idC = "C";
std::string idBS = "BS";

int preEnrolmentRetrival(UAV * C){

    std::cout << "\nC will now retrieve A's credentials.\n";

    // Wait for A's credentials
    json rsp = C->socketModule.receiveMessage();
    printJSON(rsp);

    // Check if an error occurred
    if (rsp.contains("error")) {
        std::cerr << "Error occurred: " << rsp["error"] << std::endl;
        return -1;
    }

    // Retrieve CA and RA from the msg
    unsigned char CA[PUF_SIZE];
    fromHexString(rsp["C"].get<std::string>(), CA, PUF_SIZE);
    unsigned char RA[PUF_SIZE];
    fromHexString(rsp["R"].get<std::string>(), RA, PUF_SIZE);
    std::cout << "CA : "; print_hex(CA, PUF_SIZE);
    std::cout << "RA : "; print_hex(RA, PUF_SIZE);

    // Conceals RA
    unsigned char xLock[PUF_SIZE];
    generate_random_bytes(xLock);
    std::cout << "xLock : "; print_hex(xLock, PUF_SIZE);

    unsigned char lock[PUF_SIZE];
    C->callPUF(xLock, lock);

    unsigned char secret[PUF_SIZE];
    xor_buffers(RA, lock, PUF_SIZE, secret);
    std::cout << "secret : "; print_hex(secret, PUF_SIZE);


    C->addUAV(idA, nullptr, CA, nullptr, xLock, secret);
    std::cout << "\nC has retrieved A's credentials.\n";

    return 0;
}

int supplementaryAuthenticationSup(UAV * C){

    // C will now try to connect to A
    json msg = {{"id", idC}};
    C->socketModule.sendMessage(msg);
    std::cout << "Sent ID.\n";

    // Wait for answer
    json rsp = C->socketModule.receiveMessage();
    printJSON(rsp);

    // Check if an error occurred
    if (rsp.contains("error")) {
        std::cerr << "Error occurred: " << rsp["error"] << std::endl;
        return -1;
    }

    // Retrieve NA 
    unsigned char NA[PUF_SIZE];
    fromHexString(rsp["NA"].get<std::string>(), NA, PUF_SIZE);

    // C retrieve CA from memory and recover RA
    const unsigned char * CA = C->getUAVData(idA)->getC();
    if (CA == nullptr){
        std::cout << "No challenge in memory for the requested UAV.\n";
        return 1;
    }
    std::cout << "CA : "; print_hex(CA, PUF_SIZE);

    const unsigned char * xLock = C->getUAVData(idA)->getXLock();
    std::cout << "xLock : "; print_hex(xLock, PUF_SIZE);
    const unsigned char * secret = C->getUAVData(idA)->getSecret();
    std::cout << "secret : "; print_hex(secret, PUF_SIZE);
    unsigned char lock[PUF_SIZE];
    C->callPUF(xLock,lock);
    unsigned char RA[PUF_SIZE];
    xor_buffers(lock, secret, PUF_SIZE, RA);
    std::cout << "RA : "; print_hex(RA, PUF_SIZE);

    // C then creates a nonce NC and the secret message M1 
    unsigned char gammaC[PUF_SIZE];
    unsigned char NC[PUF_SIZE];
    generate_random_bytes(gammaC);
    C->callPUF(gammaC,NC);
    std::cout << "NC : "; print_hex(NC, PUF_SIZE);

    unsigned char M1[PUF_SIZE];

    xor_buffers(RA,NC,PUF_SIZE,M1);
    std::cout << "M1 : "; print_hex(M1, PUF_SIZE);

    // C sends its ID, M1 and a hash of idC, CA, NB, RA, NA to A
    unsigned char hash1[PUF_SIZE];
    EVP_MD_CTX * ctx = initHash();
    addToHash(ctx, idC);
    addToHash(ctx, CA, PUF_SIZE);
    addToHash(ctx, NC, PUF_SIZE);
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash1);
    std::cout << "hash1 : "; print_hex(hash1, PUF_SIZE);
    
    msg = {
        {"id", idC}, 
        {"C", toHexString(CA, PUF_SIZE)}, 
        {"M1", toHexString(M1, PUF_SIZE)}, 
        {"hash1", toHexString(hash1, PUF_SIZE)}
    };
    C->socketModule.sendMessage(msg);
    std::cout << "Sent ID, CA, M1 and hash1.\n";

    // Wait for A's response 
    rsp = C->socketModule.receiveMessage();
    printJSON(rsp);

    // Check if an error occurred
    if (rsp.contains("error")) {
        std::cerr << "Error occurred: " << rsp["error"] << std::endl;
        return -1;
    }

    // B recovers M2 and hash2
    unsigned char M2[PUF_SIZE];
    fromHexString(rsp["M2"].get<std::string>(), M2, PUF_SIZE);
    unsigned char hash2[PUF_SIZE];
    fromHexString(rsp["hash2"].get<std::string>(), hash2, PUF_SIZE);

    // B retrieve RAp from M2
    unsigned char RAp[PUF_SIZE];
    xor_buffers(M2, NC, PUF_SIZE, RAp);
    std::cout << "RAp : "; print_hex(RAp, PUF_SIZE);

    // B verify the hash
    unsigned char hash2Check[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, NC, PUF_SIZE);
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx, RAp, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash2Check);
    std::cout << "hash2Check : "; print_hex(hash2Check, PUF_SIZE);

    int res = memcmp(hash2, hash2Check, PUF_SIZE) == 0;
    std::cout << "C verify A's hash : " << res << "\n";

    if(res == 0){
        std::cout << "The hashes do not correspond.\n";
        return 1;
    }

    std::cout << "A's hash has been verified. A is autenticated to C.\n";

    // B changes its values
    C->getUAVData(idA)->setX(gammaC);
    C->getUAVData(idA)->setR(RAp);

    // B sends a hash of RAp, NB, NA as an ACK
    unsigned char hash3[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, RAp, PUF_SIZE);
    addToHash(ctx, NC, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash3);
    std::cout << "hash3 : "; print_hex(hash3, PUF_SIZE);
    
    msg = {{"id", idC}, {"hash3", toHexString(hash3, PUF_SIZE)}};
    C->socketModule.sendMessage(msg);
    std::cout << "Sent ID and hash3.\n";

    // Finished
    std::cout << "\nThe two UAV autenticated each other.\n";

    return 0;
}

int main(){

    // Creation of the UAV

    UAV C = UAV(idC);

    std::cout << "The supplementary drone id is : " <<C.getId() << ".\n"; 

    // Connect to the BS to retrieve A's credentials
    C.socketModule.initiateConnection("127.0.0.1", 8080);

    // A's credential retrieval
    int ret = preEnrolmentRetrival(&C);
    if (ret == 1){
        return ret;
    }

    C.socketModule.closeConnection();

    // C will now try to connect to A
    C.socketModule.initiateConnection("127.0.0.1",8085);

    ret = supplementaryAuthenticationSup(&C);
    if (ret == 1){
        return ret;
    }

    return 0;
}