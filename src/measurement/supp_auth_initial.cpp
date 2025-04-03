#include <string>
#include <chrono> 

#include "../UAV.hpp"
#include "../puf.hpp"
#include "../utils.hpp"
#include "../SocketModule.hpp"
#include "../CycleCounter.hpp"

std::string idA = "A";
std::string idBS = "BS";

void warmup(UAV * A){
    unsigned char rand[PUF_SIZE];
    generate_random_bytes(rand);
    unsigned char out[PUF_SIZE];
    A->callPUF(rand, out);
    // print_hex(out, PUF_SIZE);
}

int supplementaryAuthenticationInitial(UAV * A){

    // Waits for C demands
    json rsp = A->socketModule.receiveMessage();
    // printJSON(rsp);

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
    // std::cout << "UAVData not found for idC.\n" << std::endl;
    // std::cout << "Initiate supplementary authentication\n" << std::endl;
    
    // A generates an nonce NA
    unsigned char NA[PUF_SIZE];
    generate_random_bytes(NA);
    // std::cout << "NA : "; print_hex(NA, PUF_SIZE);

    json msg = {{"id", idA}, {"NA", toHexString(NA, PUF_SIZE)}};

    // A sends 
    A->socketModule.sendMessage(msg);
    // std::cout << "Sent ID and NA.\n";

    // Waits for C's response 
    rsp = A->socketModule.receiveMessage();
    // printJSON(rsp);

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
    // std::cout << "RA : "; print_hex(RA, PUF_SIZE);
    
    // A retrieve NC from M1 
    unsigned char NC[PUF_SIZE];
    xor_buffers(M1, RA, PUF_SIZE, NC);
    // std::cout << "NC : "; print_hex(NC, PUF_SIZE);

    // A verify the hash
    unsigned char hash1Check[PUF_SIZE];
    EVP_MD_CTX * ctx = initHash();
    addToHash(ctx, idC);
    addToHash(ctx, CA, PUF_SIZE);
    addToHash(ctx, NC, PUF_SIZE);
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash1Check);
    // std::cout << "hash1Check : "; print_hex(hash1Check, PUF_SIZE);

    bool res = memcmp(hash1, hash1Check, PUF_SIZE) == 0;
    // std::cout << "A verify C's hash : " << res << "\n";

    if(res == 0){
        std::cout << "The autentication failed.\n";
        return res;

    }

    // std::cout << "C's hash has been verified. C is autenticated to A.\n";

    // A will now conputes the new challenge and M2
    unsigned char RAp[PUF_SIZE];
    A->callPUF(NC,RAp);
    // std::cout << "RAp : "; print_hex(RAp, PUF_SIZE);

    unsigned char M2[PUF_SIZE];
    xor_buffers(NC,RAp,PUF_SIZE,M2);
    // std::cout << "M2 : "; print_hex(M2, PUF_SIZE);

    // A sends M2, and a hash of NB, RA, RAp, NA
    unsigned char hash2[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, NC, PUF_SIZE);
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx, RAp, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash2);
    // std::cout << "hash2 : "; print_hex(hash2, PUF_SIZE);

    // TODO : send M2, and a hash of NB, RA, RAp, NA
    msg = {
        {"id", idA},
        {"M2", toHexString(M2, PUF_SIZE)},
        {"hash2", toHexString(hash2, PUF_SIZE)}
    };
    A->socketModule.sendMessage(msg);
    // std::cout << "Sent ID, M2 and hash2.\n";

    // A waits for C's ACK
    rsp = A->socketModule.receiveMessage();
    // printJSON(rsp);

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
    return 0;
}

int main() {

    CycleCounter counter;
    long long start;
    long long end;
    long long m1;

    // Creation of the UAV

    std::string SA = std::string("dc5792cd19dc40bb7433b9309809d1f1e042832e075d431d5d6c622fc5ee7544");
    unsigned char saltA[PUF_SIZE];
    fromHexString(SA, saltA, PUF_SIZE);
    UAV A = UAV(idA, saltA);
    // A.printSalt();

    // std::cout << "The initial drone id is : " <<A.getId() << ".\n"; 

    // Counters and warmup

    warmup(&A);

    // CycleCounter counter;
    // long long start;
    // long long end;


    // Initiate connection

    A.socketModule.waitForConnection(8080);
    // When the programm reaches this point, the UAV are connected

    start = counter.getCycles();
    int ret = supplementaryAuthenticationInitial(&A);
    end = counter.getCycles();
    m1 = end - start;
    if (ret != 0){
        return ret;
    }

    std::cout << "\nThe two UAV autenticated each other.\n";
    std::cout << "m1 Elapsed CPU cycles initial UAV supp authentication: " << m1 << " cycles" << std::endl;

    return 0;
}