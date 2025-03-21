/**
 * @file scenario3.cpp
 * @brief Single thread version of scenario3.
 * 
 * This file contains the third scenario. In this scenario, two drones enroll with each other and then attempt an authentication.
 * However, that authentication fails because M2 and ACK is never received. Therefore the initiator saves the concealed old challenge 
 * in memory. Then the next authentication is represented.
 */

#include <string>
#include <chrono> 

#include "UAV.hpp"
#include "puf.hpp"
#include "utils.hpp"

std::string idA = "A";
std::string idB = "B";

int enrolment(UAV * A, UAV * B){
    // A enroll with B 
    auto start = std::chrono::high_resolution_clock::now(); 

    // Computes xB
    unsigned char xB[PUF_SIZE];
    generate_random_bytes(xB, PUF_SIZE);

    // Creates B in the memory of A and save xB 
    A->addUAV(idB, xB);

    // Creates the challenge for B
    unsigned char CB[PUF_SIZE];
    A->callPUF(xB, CB);

    // Sends CB
    // TODO : send CB

    // B receive CB. It creates A in the memory of B and save CB.
    B->addUAV(idA, nullptr, CB);

    // B computes RB
    unsigned char RB[PUF_SIZE];
    B->callPUF(CB, RB);

    // B sends RB
    // TODO : send RB

    // A receive RB and saves it. 
    A->getUAVData(idB)->setR(RB);

    // A is enrolled with B
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Execution time for A to enroll with B : " << duration.count() << " microseconds" << std::endl;
 

    // B enroll with A
    start = std::chrono::high_resolution_clock::now(); 

    // Computes xA
    unsigned char xA[PUF_SIZE];
    generate_random_bytes(xA, PUF_SIZE);

    // Save xA
    B->getUAVData(idA)->setX(xA);

    // Creates the challenge for A
    unsigned char CA[PUF_SIZE];
    B->callPUF(xA, CA);

    // Sends CA
    // TODO : send CA

    // A receive CA. It save CA.
    A->getUAVData(idB)->setC(CA);

    // A computes RA
    unsigned char RA[PUF_SIZE];
    A->callPUF(CA, RA);

    // A sends RA
    // TODO : send RA

    // B receive RA and saves it. 
    B->getUAVData(idA)->setR(RA);

    // B is enrolled with A 
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Execution time for B to enroll with A : " << duration.count() << " microseconds" << std::endl;

    return 0;
}

int failedAuth(UAV * A, UAV * B){
        // A will now try to start an authentication process with B

    // A generates a nonce NA 
    unsigned char NA[PUF_SIZE];
    generate_random_bytes(NA);

    // A sends its ID and NA to B 
    // TODO : sends idA and NA

    // B receive idA and NA
    // B retrieve xA from memory and computes CA
    const unsigned char * xA_b = B->getUAVData(idA)->getX();
    if (xA_b == nullptr){
        std::cout << "No challenge in memory for this UAV.\n";
        return 1;
    }
    unsigned char CA_b[PUF_SIZE];
    B->callPUF(xA_b,CA_b);

    // B then creates a nonce NB and the secret message M1 
    unsigned char gammaB[PUF_SIZE];
    unsigned char NB_b[PUF_SIZE];
    generate_random_bytes(gammaB);
    B->callPUF(gammaB,NB_b);

    unsigned char M1[PUF_SIZE];
    const unsigned char * RA_b = B->getUAVData(idA)->getR();
    if (RA_b == nullptr){
        std::cout << "No response in memory for this UAV.\n";
        return 1;
    }
    xor_buffers(RA_b,NB_b,PUF_SIZE,M1);

    // B sends its ID, M1 and a hash of CA, NB, RA, NA to A
    unsigned char hash1[PUF_SIZE];
    EVP_MD_CTX * ctx = initHash();
    addToHash(ctx, CA_b, PUF_SIZE);
    addToHash(ctx, NB_b, PUF_SIZE);
    addToHash(ctx, RA_b, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash1);
    //TODO : send idB, M1 and hash1

    // A receive idB, M1 and hash1
    // A computes RA using CA in memory
    const unsigned char * CA_a = A->getUAVData(idB)->getC();
    if (CA_a == nullptr){
        std::cout << "No expected challenge in memory for this UAV.\n";
        return 1;
    }
    unsigned char RA_a[PUF_SIZE];
    A->callPUF(CA_a,RA_a);

    // A retrieve NB from M1 
    unsigned char NB_a[PUF_SIZE];
    xor_buffers(M1, RA_a, PUF_SIZE, NB_a);

    // A verify the hash
    unsigned char hash1Check[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, CA_a, PUF_SIZE);
    addToHash(ctx, NB_a, PUF_SIZE);
    addToHash(ctx, RA_a, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash1Check);

    bool res = memcmp(hash1, hash1Check, PUF_SIZE) == 0;
    std::cout << "Are the two hashes the same : " << res << "\n";

    if(res == 0){
        return 1;
    }

    std::cout << "B is autenticated to A\n";


    unsigned char RAp_a[PUF_SIZE];
    A->callPUF(NB_a,RAp_a);

    unsigned char M2[PUF_SIZE];
    xor_buffers(NB_a,RAp_a,PUF_SIZE,M2);

    // A sends M2, and a hash of NB, RA, RAp, NA
    unsigned char hash2[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, NB_a, PUF_SIZE);
    addToHash(ctx, RA_a, PUF_SIZE);
    addToHash(ctx, RAp_a, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash2);
    // TODO : (NOT) send M2, and a hash of NB, RA, RAp, NA

    // B do not receive M2 and the hash
    // A never receive the ACK, may try to resend th message but it never arrives.

    // A reach a timeout, therefore it save concealed CAOld
    unsigned char xLock[PUF_SIZE];
    generate_random_bytes(xLock);
    unsigned char lock[PUF_SIZE];
    A->callPUF(xLock, lock);
    unsigned char concealedCA[PUF_SIZE];
    xor_buffers(CA_a,lock,PUF_SIZE,concealedCA);
    A->getUAVData(idB)->setXLock(xLock);
    A->getUAVData(idB)->setSecret(concealedCA);

    // Then A saves the new challenge in CA as if everything went smoothly
    A->getUAVData(idB)->setC(NB_a);

    return 0;
}

int autentication(UAV * A, UAV * B){
    // A will now try to start an authentication process with B
    auto start = std::chrono::high_resolution_clock::now(); 

    // A generates a nonce NA 
    unsigned char NA[PUF_SIZE];
    generate_random_bytes(NA);

    // A sends its ID and NA to B 
    // TODO : sends idA and NA

    // B receive idA and NA
    // B retrieve xA from memory and computes CA
    const unsigned char * xA_b = B->getUAVData(idA)->getX();
    if (xA_b == nullptr){
        std::cout << "No challenge in memory for the requested UAV.\n";
        return 1;
    }

    unsigned char CA_b[PUF_SIZE];
    B->callPUF(xA_b,CA_b);

    // B then creates a nonce NB and the secret message M1 
    unsigned char gammaB[PUF_SIZE];
    unsigned char NB_b[PUF_SIZE];
    generate_random_bytes(gammaB);
    B->callPUF(gammaB,NB_b);

    unsigned char M1[PUF_SIZE];
    const unsigned char * RA_b = B->getUAVData(idA)->getR();
    if (RA_b == nullptr){
        std::cout << "No response in memory for the requested UAV.\n";
        return 1;
    }
    xor_buffers(RA_b,NB_b,PUF_SIZE,M1);

    // B sends its ID, M1 and a hash of CA, NB, RA, NA to A
    unsigned char hash1[PUF_SIZE];
    EVP_MD_CTX * ctx = initHash();
    addToHash(ctx, CA_b, PUF_SIZE);
    addToHash(ctx, NB_b, PUF_SIZE);
    addToHash(ctx, RA_b, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash1);
    //TODO : send idB, M1 and hash1

    // A receive idB, M1 and hash1
    // A computes RA using CA in memory
    const unsigned char * CA_a = A->getUAVData(idB)->getC();
    if (CA_a == nullptr){
        std::cout << "No expected challenge in memory for this UAV.\n";
        return 1;
    }
    unsigned char RA_a[PUF_SIZE];
    A->callPUF(CA_a,RA_a);
    
    // A retrieve NB from M1 
    unsigned char NB_a[PUF_SIZE];
    xor_buffers(M1, RA_a, PUF_SIZE, NB_a);

    // A verify the hash
    unsigned char hash1Check[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, CA_a, PUF_SIZE);
    addToHash(ctx, NB_a, PUF_SIZE);
    addToHash(ctx, RA_a, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash1Check);

    bool res = memcmp(hash1, hash1Check, PUF_SIZE) == 0;
    std::cout << "Are the two hashes the same : " << res << "\n";

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
        unsigned char CAOld_a[PUF_SIZE]; 
        xor_buffers(lock, secret, PUF_SIZE, CAOld_a);

        // A will calculate the old response 
        unsigned char RAOld_a[PUF_SIZE];
        A->callPUF(CAOld_a, RAOld_a);

        // A will deduce NB from the old response
        unsigned char NBOld[PUF_SIZE];
        xor_buffers(M1, RAOld_a, PUF_SIZE, NBOld);

        // A now tries to verify the hash with this value
        ctx = initHash();
        addToHash(ctx, CAOld_a, PUF_SIZE);
        addToHash(ctx, NBOld, PUF_SIZE);
        addToHash(ctx, RAOld_a, PUF_SIZE);
        addToHash(ctx, NA, PUF_SIZE);
        calculateHash(ctx, hash1Check);

        res = memcmp(hash1, hash1Check, PUF_SIZE) == 0;
        if (res == 0){
            std::cout << "Even with the old challenge, autenntication has failed.\n";
            return 1;
        }
        std::cout << "B has been autenticated by A with the old challenge.\n";

        // A will now change the values to be the one obtained of the old challenge
        A->getUAVData(idB)->setC(CAOld_a);
        memcpy(RA_a, RAOld_a, PUF_SIZE);
        memcpy(NB_a, NBOld, PUF_SIZE);

    }

    std::cout << "B is autenticated to A\n";


    unsigned char RAp_a[PUF_SIZE];
    A->callPUF(NB_a,RAp_a);

    unsigned char M2[PUF_SIZE];
    xor_buffers(NB_a,RAp_a,PUF_SIZE,M2);

    // A sends M2, and a hash of NB, RA, RAp, NA
    unsigned char hash2[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, NB_a, PUF_SIZE);
    addToHash(ctx, RA_a, PUF_SIZE);
    addToHash(ctx, RAp_a, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash2);
    // TODO : send M2, and a hash of NB, RA, RAp, NA
    
    // B receive M2 and the hash

    // B retrieve RAp from M2
    unsigned char RAp_b[PUF_SIZE];
    xor_buffers(M2, NB_b, PUF_SIZE, RAp_b);

    // B verify the hash
    unsigned char hash2Check[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, NB_b, PUF_SIZE);
    addToHash(ctx, RA_b, PUF_SIZE);
    addToHash(ctx, RAp_b, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash2Check);

    res = memcmp(hash2, hash2Check, PUF_SIZE) == 0;
    std::cout << "Are the two hashes the same : " << res << "\n";

    if(res == 0){
        return 1;
    }

    std::cout << "A is autenticated to B\n";

    // B changes its values
    B->getUAVData(idA)->setX(gammaB);
    B->getUAVData(idA)->setR(RAp_b);

    // B sends a hash of RAp, NB, NA as an ack
    unsigned char hash3[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, RAp_b, PUF_SIZE);
    addToHash(ctx, NB_b, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash3);
    // TODO : send a hash of RAp, NB, NA as an ack

    // A receive the hash and verify it
    unsigned char hash3Check[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, RAp_a, PUF_SIZE);
    addToHash(ctx, NB_a, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash3Check);

    res = memcmp(hash3, hash3Check, PUF_SIZE) == 0;
    std::cout << "Are the two hashes the same : " << res << "\n";

    if(res == 0){
        return 1;
    }

    std::cout << "The ACK hash is verified\n";
    
    // A changes its values
    A->getUAVData(idB)->setC(NB_a);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Execution time for A to authenticate with B : " << duration.count() << " microseconds" << std::endl;
    return 0;
}

int main(){
    // Creation of the two drones

    UAV A = UAV(idA);
    UAV B = UAV(idB);

    std::cout << "The first drone id is : " <<A.getId() << ".\n"; 
    std::cout << "The second drone id is : " <<B.getId() << ".\n"; 

    int ret = enrolment(&A, &B);
    if (ret == 1){
        return 1;
    }
    std::cout << "The two drones are enrolled\n";

    ret = failedAuth(&A, &B);
    if (ret == 1){
        return 1;
    }
    std::cout << "After communication problems, A and B are desynchronized. A expects the new challenge but B still has the old one.\n";

    ret = autentication(&A, &B);
    if (ret == 1){
        return 1;
    }

    return 0;
}

// std::cout << "\nPrinting the recovered saved old values\n";
// std::cout << "CAOld_a (size " << PUF_SIZE << "):\n"; print_hex(CAOld_a, PUF_SIZE);
// std::cout << "xLock (size " << PUF_SIZE << "):\n"; print_hex(xLock, PUF_SIZE);
// std::cout << "secret (size " << PUF_SIZE << "):\n"; print_hex(secret, PUF_SIZE);