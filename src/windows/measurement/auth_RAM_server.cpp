#include <string>
#include <chrono> 

#include "../UAV.hpp"
#include "../puf.hpp"
#include "../utils.hpp"
#include "../SocketModule.hpp"

std::string idA = "A";
std::string idB = "B";
bool server = true;

int enrolment_server(UAV * B){
    // std::cout << "\nEnrolment process begins.\n";

    // B waits for B's message  (with CB)
    json rsp = B->socketModule.receiveMessage();
    // printJSON(rsp);

    // Check if an error occurred
    if (rsp.contains("error")) {
        std::cerr << "Error occurred: " << rsp["error"] << std::endl;
        return -1;
    }

    // B receive CB. It creates A in the memory of B and save CB.
    unsigned char CB[PUF_SIZE];
    if(!rsp.contains("CB")){
        std::cerr << "Error occurred: no member CB" << std::endl;
        return 1;
    }
    fromHexString(rsp["CB"].get<std::string>(), CB, PUF_SIZE);
    B->addUAV(idA, nullptr, CB);

    // B computes RB
    unsigned char RB[PUF_SIZE];
    B->callPUF(CB, RB);
    // std::cout << "RB : "; print_hex(RB, PUF_SIZE);

    // B sends RB
    json msg = {{"id", idB}, {"RB", toHexString(RB, PUF_SIZE)}};
    B->socketModule.sendMessage(msg);
    // std::cout << "Sent RB.\n";

    // B enroll with A
    // Computes xA
    unsigned char xA[PUF_SIZE];
    generate_random_bytes(xA, PUF_SIZE);
    // std::cout << "xA : "; print_hex(xA, PUF_SIZE);

    // Save xA
    B->getUAVData(idA)->setX(xA);

    // Creates the challenge for A
    unsigned char CA[PUF_SIZE];
    B->callPUF(xA, CA);
    // std::cout << "CA : "; print_hex(CA, PUF_SIZE);

    // Sends CA
    msg = {{"id", idB}, {"CA", toHexString(CA, PUF_SIZE)}};
    B->socketModule.sendMessage(msg);
    // std::cout << "Sent CA.\n";

    // B receive RA and saves it. 
    rsp = B->socketModule.receiveMessage();
    // printJSON(rsp);

    // Check if an error occurred
    if (rsp.contains("error")) {
        std::cerr << "Error occurred: " << rsp["error"] << std::endl;
        return -1;
    }

    unsigned char RA[PUF_SIZE];
    if(!rsp.contains("RA")){
        std::cerr << "Error occurred: no member RA" << std::endl;
        return 1;
    }
    fromHexString(rsp["RA"].get<std::string>(), RA, PUF_SIZE);
    B->getUAVData(idA)->setR(RA);
    
    return 0;
}

int autentication_server(UAV * B){
    // The client initiate the autentication process
    // std::cout << "\nAutentication process begins.\n";

    // B receive the initial message
    json rsp = B->socketModule.receiveMessage();
    // printJSON(rsp);

    // Check if an error occurred
    if (rsp.contains("error")) {
        std::cerr << "Error occurred: " << rsp["error"] << std::endl;
        return -1;
    }

    // B recover M0
    unsigned char M0[PUF_SIZE];
    if(!rsp.contains("M0")){
        std::cerr << "Error occurred: no member M0" << std::endl;
        return 1;
    }
    fromHexString(rsp["M0"].get<std::string>(), M0, PUF_SIZE);
    
    // B retrieve xA from memory and computes CA
    const unsigned char * xA = B->getUAVData(idA)->getX();
    if (xA == nullptr){
        // std::cout << "No challenge in memory for the requested UAV.\n";
        return 1;
    }
    // std::cout << "xA : "; print_hex(xA, PUF_SIZE);
    
    unsigned char CA[PUF_SIZE];
    B->callPUF(xA,CA);
    // std::cout << "CA : "; print_hex(CA, PUF_SIZE);
    
    unsigned char NA[PUF_SIZE];
    xor_buffers(M0, CA, PUF_SIZE, NA);
    // std::cout << "NA : "; print_hex(NA, PUF_SIZE);

    // B then creates a nonce NB and the secret message M1 
    unsigned char gammaB[PUF_SIZE];
    unsigned char NB[PUF_SIZE];
    generate_random_bytes(gammaB);
    B->callPUF(gammaB,NB);
    // std::cout << "NB : "; print_hex(NB, PUF_SIZE);

    unsigned char M1[PUF_SIZE];
    const unsigned char * RA = B->getUAVData(idA)->getR();
    if (RA == nullptr){
        // std::cout << "No response in memory for the requested UAV.\n";
        return 1;
    }
    // std::cout << "RA : "; print_hex(RA, PUF_SIZE);
    xor_buffers(RA,NA,PUF_SIZE,M1);
    xor_buffers(M1,NB,PUF_SIZE,M1);
    // std::cout << "M1 : "; print_hex(M1, PUF_SIZE);

    // B sends its ID, M1 and a hash of CA, NB, RA, NA to A
    unsigned char hash1[PUF_SIZE];
    EVP_MD_CTX * ctx = initHash();
    addToHash(ctx, CA, PUF_SIZE);
    addToHash(ctx, NB, PUF_SIZE);
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash1);
    // std::cout << "hash1 : "; print_hex(hash1, PUF_SIZE);
    
    json msg = {
        {"id", idB}, 
        {"M1", toHexString(M1, PUF_SIZE)}, 
        {"hash1", toHexString(hash1, PUF_SIZE)}
    };
    B->socketModule.sendMessage(msg);
    // std::cout << "Sent ID, M1 and hash1.\n";

    // B waits for A response (M2)
    rsp = B->socketModule.receiveMessage();
    // printJSON(rsp);

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
    xor_buffers(M2, NB, PUF_SIZE, RAp);
    // std::cout << "RAp : "; print_hex(RAp, PUF_SIZE);

    // B verify the hash
    unsigned char hash2Check[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, NB, PUF_SIZE);
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx, RAp, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash2Check);
    // std::cout << "hash2Check : "; print_hex(hash2Check, PUF_SIZE);

    int res = memcmp(hash2, hash2Check, PUF_SIZE) == 0;
    // std::cout << "B verify A's hash : " << res << "\n";

    if(res == 0){
        // std::cout << "The hashes do not correspond.\n";
        return 1;
    }

    // std::cout << "A's hash has been verified. A is autenticated to B.";

    // B changes its values
    B->getUAVData(idA)->setX(gammaB);
    B->getUAVData(idA)->setR(RAp);

    // B sends a hash of RAp, NB, NA as an ACK
    unsigned char hash3[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, RAp, PUF_SIZE);
    addToHash(ctx, NB, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash3);
    // std::cout << "hash3 : "; print_hex(hash3, PUF_SIZE);
    
    msg = {{"id", idB}, {"hash3", toHexString(hash3, PUF_SIZE)}};
    B->socketModule.sendMessage(msg);
    // std::cout << "Sent ID and hash3.\n";

    // Finished
    std::cout << "\nThe two UAV autenticated each other.\n";

    return 0;
}

int main(){

    // Creation of the UAV

    UAV B = UAV(idB);

    // std::cout << "The server drone id is : " <<B.getId() << ".\n"; 

    B.socketModule.waitForConnection(8080);

    // When the programm reaches this point, the UAV are connected

    int ret = enrolment_server(&B);
    if (ret == 1){
        return ret;
    }

    ret = autentication_server(&B);
    if (ret == 1){
        return ret;
    }
    
    B.socketModule.closeConnection();

    return 0;
}
