

#include "../UAV.hpp"
#include "../puf.hpp"
#include "../utils.hpp"
#include "../SocketModule.hpp"

std::string idW = "B";

/**
 * @brief W tries to impersonate a trusted drone B.
 * 
 * @param W 
 * @return int 
 */
int impersonation_server(UAV * W){
    
    // Waits for A to contact
    json rsp = W->socketModule.receiveMessage();
    printJSON(rsp);

    // Check if an error occurred
    if (rsp.contains("error")) {
        std::cerr << "Error occurred: " << rsp["error"] << std::endl;
        return -1;
    }

    if(!rsp.contains("id")){
        std::cerr << "Error occurred: no member id" << std::endl;
        return 1;
    }
    std::string idA = rsp["id"].get<std::string>(); 

    unsigned char NA[PUF_SIZE];
    if(!rsp.contains("NA")){
        std::cerr << "Error occurred: no member NA" << std::endl;
        return 1;
    }
    fromHexString(rsp["NA"].get<std::string>(), NA, PUF_SIZE);   

    // W will now prepare a bogus message
    unsigned char NB[PUF_SIZE];
    generate_random_bytes(NB);
    unsigned char CA[PUF_SIZE];
    generate_random_bytes(CA);
    unsigned char RA[PUF_SIZE];
    generate_random_bytes(RA);
    unsigned char M1[PUF_SIZE];
    xor_buffers(RA, NB, PUF_SIZE, M1);

    unsigned char hash1[PUF_SIZE];
    EVP_MD_CTX * ctx = initHash();
    addToHash(ctx, CA);
    addToHash(ctx, RA);
    addToHash(ctx, NB);
    addToHash(ctx, NA);
    calculateHash(ctx, hash1);

    // W sends the bogus message
    json msg = {
        {"id", idW}, 
        {"M1", toHexString(M1, PUF_SIZE)}, 
        {"hash1", toHexString(hash1, PUF_SIZE)}
    };
    W->socketModule.sendMessage(msg);

    // W waits for a reponse 
    rsp = W->socketModule.receiveMessage();
    printJSON(rsp);

    if(!rsp.contains("id")){
        std::cerr << "Error occurred: no member id" << std::endl;
        return 1;
    }

    return 0;
}

int main() {

    // Creation of the UAV

    UAV W = UAV(idW);

    // Connection between the UAVs

    W.socketModule.waitForConnection(8080);

    // Impersonation attack

    int ret = impersonation_server(&W);
    if(ret != 0){
        std::cout << "Impersonation attack failed ✅" << std::endl;
    }
    else{
        std::cout << "Impersonation attack succeeded ❌" << std::endl;
    }
    
    return 0;
}