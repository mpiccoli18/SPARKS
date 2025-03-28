// #include <string>

#include "../UAV.hpp"
#include "../puf.hpp"
#include "../utils.hpp"
#include "../SocketModule.hpp"

std::string idA = "A";
std::string idB = "B";

int impersonation_client(UAV * A){

    // A sends its id and a random nonce NA to B
    unsigned char NA[PUF_SIZE];
    generate_random_bytes(NA);

    json msg = {{"id", idA}, {"NA", toHexString(NA, PUF_SIZE)}};
    A->socketModule.sendMessage(msg);

    // A waits for B's response 
    json rsp = A->socketModule.receiveMessage();
    printJSON(rsp);

    // Check if an error occurred
    if (rsp.contains("error")) {
        std::cerr << "Error occurred: " << rsp["error"] << std::endl;
        return -1;
    }

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

    // A calculates RB with CB in memory, recover NB and verify hash
    const unsigned char * CA = A->getUAVData(idB)->getC();
    unsigned char RA[PUF_SIZE];
    A->callPUF(CA, RA);

    unsigned char NB[PUF_SIZE];
    xor_buffers(M1, RA, PUF_SIZE, NB);

    unsigned char hash1Check[PUF_SIZE];
    EVP_MD_CTX * ctx = initHash();
    addToHash(ctx, CA);
    addToHash(ctx, RA);
    addToHash(ctx, NB);
    addToHash(ctx, NA);
    calculateHash(ctx, hash1Check);

    bool res = memcmp(hash1, hash1Check, PUF_SIZE) == 0; // should fail
    std::cout << "A verify C's hash : " << res << "\n";

    if(res == 0){
        std::cout << "The autentication failed.\n";
        return 1;
    }

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

    unsigned char CB[PUF_SIZE];
    generate_random_bytes(CB);
    A.addUAV(idB, nullptr, CB);

    // Connection between the UAVs

    A.socketModule.initiateConnection(ip, 8080);

    // Impersonation attack

    int ret = impersonation_client(&A);
    if(ret != 0){
        std::cout << "Impersonation attack failed ✅" << std::endl;
    }
    else{
        std::cout << "Impersonation attack succeeded ❌" << std::endl;
    }

    return 0;
}