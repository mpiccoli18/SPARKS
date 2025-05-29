/**
 * @file 4_supp_auth_supplementary.cpp
 * @brief This file's goal is to measure the overheads of the supplementary authentication key function. The output is the total time taken by the function to execute. 
 * If the project is compiled with -DMEASUREMENTS, The output also includes the active and idle time of the function execution.
 * 
 */

#include <chrono> 
#include <thread>

#include "../UAV.hpp"
#include "../puf.hpp"
#include "../utils.hpp"
#include "../SocketModule.hpp"
#include "../CycleCounter.hpp"

std::string idA = "A";
std::string idC = "C";
std::string idBS = "BS";

CycleCounter counter;

long long start;
long long start_init;
long long end;
long long m1;
long long idlcycles = 0;
long long opCycles = 0;

void warmup(UAV * A){
    unsigned char rand[PUF_SIZE];
    generate_random_bytes(rand);
    unsigned char out[PUF_SIZE];
    A->callPUF(rand, out);
    // print_hex(out, PUF_SIZE);
}

int supplementaryAuthenticationSup(UAV * C){
    start = counter.getCycles();
    // C will now try to connect to A
    json msg = {{"id", idC}};
    C->socketModule.sendMessage(msg);
    // std::cout << "Sent ID.\n";
    end = counter.getCycles();;
    opCycles += end - start;
    start = counter.getCycles();

    // Wait for answer
    json rsp = C->socketModule.receiveMessage();
    // printJSON(rsp);
    end = counter.getCycles();
    idlcycles += end - start;
    start = counter.getCycles();

    // Check if an error occurred
    if (rsp.contains("error")) {
        std::cerr << "Error occurred: " << rsp["error"] << std::endl;
        return -1;
    }

    // Retrieve NA 
    unsigned char NA[PUF_SIZE];
    if(!rsp.contains("NA")){
        std::cerr << "Error occurred: no member NA" << std::endl;
        return 1;
    }
    fromHexString(rsp["NA"].get<std::string>(), NA, PUF_SIZE);

    // C retrieve CA from memory and recover RA
    const unsigned char * CA = C->getUAVData(idA)->getC();
    if (CA == nullptr){
        std::cout << "No challenge in memory for the requested UAV.\n";
        return 1;
    }
    // std::cout << "CA : "; print_hex(CA, PUF_SIZE);

    const unsigned char * xLock = C->getUAVData(idA)->getXLock();
    // std::cout << "xLock : "; print_hex(xLock, PUF_SIZE);
    const unsigned char * secret = C->getUAVData(idA)->getSecret();
    // std::cout << "secret : "; print_hex(secret, PUF_SIZE);
    unsigned char lock[PUF_SIZE];
    C->callPUF(xLock,lock);
    unsigned char RA[PUF_SIZE];
    xor_buffers(lock, secret, PUF_SIZE, RA);
    // std::cout << "RA : "; print_hex(RA, PUF_SIZE);

    // C then creates a nonce NC and the secret message M1 
    unsigned char gammaC[PUF_SIZE];
    unsigned char NC[PUF_SIZE];
    generate_random_bytes(gammaC);
    C->callPUF(gammaC,NC);
    // std::cout << "NC : "; print_hex(NC, PUF_SIZE);

    unsigned char M1[PUF_SIZE];

    xor_buffers(RA,NC,PUF_SIZE,M1);
    // std::cout << "M1 : "; print_hex(M1, PUF_SIZE);

    // C sends its ID, M1 and a hash of idC, CA, NB, RA, NA to A
    unsigned char hash1[PUF_SIZE];
    EVP_MD_CTX * ctx = initHash();
    addToHash(ctx, idC);
    addToHash(ctx, CA, PUF_SIZE);
    addToHash(ctx, NC, PUF_SIZE);
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash1);
    // std::cout << "hash1 : "; print_hex(hash1, PUF_SIZE);
    
    msg = {
        {"id", idC}, 
        {"CA", toHexString(CA, PUF_SIZE)}, 
        {"M1", toHexString(M1, PUF_SIZE)}, 
        {"hash1", toHexString(hash1, PUF_SIZE)}
    };
    C->socketModule.sendMessage(msg);
    // std::cout << "Sent ID, CA, M1 and hash1.\n";
    end = counter.getCycles();
    opCycles += end - start;
    start = counter.getCycles();

    // Wait for A's response 
    rsp = C->socketModule.receiveMessage();
    // printJSON(rsp);
    end = counter.getCycles();
    idlcycles += end - start;
    start = counter.getCycles();

    // Check if an error occurred
    if (rsp.contains("error")) {
        std::cerr << "Error occurred: " << rsp["error"] << std::endl;
        return -1;
    }

    // B recovers M2 and hash2
    unsigned char M2[PUF_SIZE];
    if(!rsp.contains("M2")){
        std::cerr << "Error occurred: no member M2" << std::endl;
        return 1;
    }
    fromHexString(rsp["M2"].get<std::string>(), M2, PUF_SIZE);
    unsigned char hash2[PUF_SIZE];
    if(!rsp.contains("hash2")){
        std::cerr << "Error occurred: no member hash2" << std::endl;
        return 1;
    }
    fromHexString(rsp["hash2"].get<std::string>(), hash2, PUF_SIZE);

    // B retrieve RAp from M2
    unsigned char RAp[PUF_SIZE];
    xor_buffers(M2, NC, PUF_SIZE, RAp);
    // std::cout << "RAp : "; print_hex(RAp, PUF_SIZE);

    // B verify the hash
    unsigned char hash2Check[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, NC, PUF_SIZE);
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx, RAp, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash2Check);
    // std::cout << "hash2Check : "; print_hex(hash2Check, PUF_SIZE);

    int res = memcmp(hash2, hash2Check, PUF_SIZE) == 0;
    // std::cout << "C verify A's hash : " << res << "\n";

    if(res == 0){
        std::cout << "The hashes do not correspond.\n";
        return 1;
    }

    // std::cout << "A's hash has been verified. A is autenticated to C.\n";

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
    // std::cout << "hash3 : "; print_hex(hash3, PUF_SIZE);
    
    msg = {{"id", idC}, {"hash3", toHexString(hash3, PUF_SIZE)}};
    C->socketModule.sendMessage(msg);
    // std::cout << "Sent ID and hash3.\n";
    end = counter.getCycles();
    opCycles += end - start;

    // Finished
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Error: No IP address provided. Please provide the IP as argument." << std::endl;
        return 1;  // Exit with an error code
    }

    const char* ip = argv[1];  // Read IP from command-line argument

    // Creation of the UAV

    std::string SC = std::string("c310d59037f5dd19b7158b96583e1b2f5f421ee6489972b791c5d04b520783ef");
    unsigned char saltC[PUF_SIZE];
    fromHexString(SC, saltC, PUF_SIZE);
    UAV C = UAV(idC, saltC);
    // C.printSalt();

    
    std::string CAstr = std::string("a47af74e3f14bb6a62b2268acf4cd538d82e57f53946d4dee11f69d3f2c0a3f5");
    unsigned char CA[PUF_SIZE];
    fromHexString(CAstr, CA, PUF_SIZE);
    
    std::string xLstr = std::string("74228aa128afcecae551af5beb1ba0983fcb244ca7631e4f70be22b8732864dd");
    unsigned char xL[PUF_SIZE];
    fromHexString(xLstr, xL, PUF_SIZE);
    // std::cout << "xL : "; print_hex(xL, PUF_SIZE); std::cout << std::endl;
    
    std::string secretStr = std::string("15a5796e0d20e8288d6faef881969ed81677d628e6d500d5ba8f67906daa1e29");
    unsigned char secret[PUF_SIZE];
    fromHexString(secretStr, secret, PUF_SIZE);
    // std::cout << "secret : "; print_hex(secret, PUF_SIZE); std::cout << std::endl;
    
    C.addUAV(idA, nullptr, CA, nullptr, xL, secret);
    
    warmup(&C);
    // Connect to the BS to retrieve A's credentials
    C.socketModule.initiateConnection(ip, 8080);
    
    start_init = counter.getCycles();
    int ret = supplementaryAuthenticationSup(&C);
    end = counter.getCycles();
    m1 = end - start_init;
    if (ret != 0){
        return ret;
    }

    std::cout << "\nThe two UAV autenticated each other.\n";
    std::cout << "m1 Elapsed CPU cycles supp UAV supp authentication: " << m1 << " cycles" << std::endl;
    std::cout << "operational Elapsed CPU cycles supp UAV supp authentication: " << opCycles << " cycles" << std::endl;
    std::cout << "idle Elapsed CPU cycles supp UAV supp authentication: " << idlcycles << " cycles" << std::endl;

    return 0;
}