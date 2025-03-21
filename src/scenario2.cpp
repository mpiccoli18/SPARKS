/**
 * @file scenario2.cpp
 * @brief Single thread version of scenario2.
 * 
 * This file contains the second scenario. In this scenario, two drones enroll with each other, one of them 
 * partake in the supplementary autentication enrolment and then attempt a supplementary authentication.
 */

#include <string>
#include <chrono> 

#include "UAV.hpp"
#include "puf.hpp"
#include "utils.hpp"

std::string idA = "A";
std::string idB = "B";
std::string idC = "C";


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

int supplementaryEnrollment(UAV * A, UAV * C, puf * BS_PUF){
    // A will now enroll with the BS
    // First we creates the BS PUF

    // The BS creates a list of 5 random numbers
    unsigned char lx[5][PUF_SIZE];
    for(int i = 0; i < 5; i++){
        generate_random_bytes(lx[i]);
    }

    // These random challenges are transformed into challenges using BS PUF
    unsigned char lC[5][PUF_SIZE];
    for(int i = 0; i < 5; i++){
        BS_PUF->process(lx[i], PUF_SIZE, lC[i]);
    }

    // A now creates the responses for those challenges
    unsigned char lR[5][PUF_SIZE];
    for(int i = 0; i < 5; i++){
        A->callPUF(lC[i], lR[i]);
    }

    // A supplementary enrolment process is completed
    
    // C now wants to enroll with A using the supplementary enrolment

    // BS will select a random number from lx and retrieve the challenge
    unsigned char CA[PUF_SIZE];
    BS_PUF->process(lx[0],PUF_SIZE,CA);

    // BS will transmit CA and RA to C
    unsigned char CA_c[PUF_SIZE];
    memcpy(CA_c, CA, PUF_SIZE);
    unsigned char RA_c[PUF_SIZE];
    memcpy(RA_c,lR[0],PUF_SIZE);
    
    // C obfuscate RA 
    unsigned char xLock[PUF_SIZE];
    generate_random_bytes(xLock);
    unsigned char lock[PUF_SIZE];
    C->callPUF(xLock,lock);
    unsigned char secret[PUF_SIZE];
    xor_buffers(RA_c,lock,PUF_SIZE,secret);

    // Then C only remembers secret, lock, CA
    C->addUAV(idA, nullptr, CA_c, nullptr, xLock, secret);
    memset(lock, 0, PUF_SIZE);
    memset(RA_c, 0, PUF_SIZE);
    
    return 0;
}

int supplementaryAutentication(UAV * C, UAV * A){
    // C now initiate an autentication with A
    // C sends its ID
    //TODO : send ID
    // A doesn't know C, so it initiate the auth process on its size
    unsigned char NA[PUF_SIZE];
    generate_random_bytes(NA);
    // TODO : send ID and NA
    
    // C receive the auth demand, as expected
    // Will now recover CA, RA from it's memory
    const unsigned char * xLock = C->getUAVData(idA)->getXLock();
    if (xLock == nullptr){
        std::cout << "No xLock value in memory for this UAV.\n";
        return 1;
    }
    unsigned char lock[PUF_SIZE];
    const unsigned char * secret = C->getUAVData(idA)->getSecret();
    if (secret == nullptr){
        std::cout << "No secret value in memory for this UAV.\n";
        return 1;
    }
    const unsigned char * CA_c = C->getUAVData(idA)->getC();
    if (CA_c == nullptr){
        std::cout << "No challenge in memory for this UAV.\n";
        return 1;
    }
    unsigned char RA_c[PUF_SIZE];

    C->callPUF(xLock,lock);
    xor_buffers(lock, secret, PUF_SIZE, RA_c);

    // It generates its nonce
    unsigned char gammaC[PUF_SIZE];
    generate_random_bytes(gammaC);
    unsigned char NC_c[PUF_SIZE];
    C->callPUF(gammaC, NC_c);

    // C them creates M1
    unsigned char M1[PUF_SIZE];
    xor_buffers(RA_c, NC_c, PUF_SIZE, M1);
    
    // C then sends idC, CA, M1 and the hash of idC, CA, NC, RA, NA
    unsigned char hash1[PUF_SIZE];
    EVP_MD_CTX * ctx = initHash();
    addToHash(ctx,idC);
    addToHash(ctx,CA_c, PUF_SIZE);
    addToHash(ctx,NC_c, PUF_SIZE);
    addToHash(ctx,RA_c, PUF_SIZE);
    addToHash(ctx,NA, PUF_SIZE);
    calculateHash(ctx, hash1);
    // TODO : send

    // A receive the message
    // A computes RA
    unsigned char RA_a[PUF_SIZE];
    A->callPUF(CA_c, RA_a);

    // A retrieve NC from M1
    unsigned char NC_a[PUF_SIZE];
    xor_buffers(M1, RA_a, PUF_SIZE, NC_a);

    // A verify the hash
    unsigned char hash1Check[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx,idC);
    addToHash(ctx,CA_c, PUF_SIZE);
    addToHash(ctx,NC_a, PUF_SIZE);
    addToHash(ctx,RA_a, PUF_SIZE);
    addToHash(ctx,NA, PUF_SIZE);
    calculateHash(ctx, hash1Check);

    bool res = memcmp(hash1, hash1Check, PUF_SIZE) == 0;
    std::cout << "Are the two hashes the same for M1 ? : " << res << "\n";

    if(res == 0){
        return 1;
    }

    std::cout << "C is autenticated to A\n";

    // A will now creates the new response and message M2
    unsigned char RAp_a[PUF_SIZE];
    A->callPUF(NC_a,RAp_a);
    unsigned char M2[PUF_SIZE];
    xor_buffers(NC_a, RAp_a, PUF_SIZE, M2);

    // A will now send idA, M2 and a hash of NC, RA, RAp, NA to C
    unsigned char hash2[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx,NC_a, PUF_SIZE);
    addToHash(ctx,RA_a, PUF_SIZE);
    addToHash(ctx,RAp_a, PUF_SIZE);
    addToHash(ctx,NA, PUF_SIZE);
    calculateHash(ctx, hash2);
    // TODO : send

    // C receive the message 
    // C retrieve RAp
    unsigned char RAp_c[PUF_SIZE];
    xor_buffers(M2, NC_c, PUF_SIZE, RAp_c);

    // C verify the hash
    unsigned char hash2Check[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, NC_c, PUF_SIZE);
    addToHash(ctx, RA_c, PUF_SIZE);
    addToHash(ctx, RAp_c, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash2Check);

    res = memcmp(hash2, hash2Check, PUF_SIZE) == 0;
    std::cout << "Are the two hashes the same for M2 ? : " << res << "\n";

    if(res == 0){
        return 1;
    }

    std::cout << "A is autenticated to C\n";

    // C changes its values 
    C->getUAVData(idA)->setX(gammaC);
    C->getUAVData(idA)->setR(RAp_c);
    C->getUAVData(idA)->setSecret(nullptr);
    C->getUAVData(idA)->setXLock(nullptr);

    // C then sends an ACk to A as a hash of RAp, NC, CA, NA
    unsigned char hash3[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, RAp_c, PUF_SIZE);
    addToHash(ctx, NC_c, PUF_SIZE);
    addToHash(ctx, CA_c, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash3);
    // TODO : send
    
    // A receive the ACK and verify the hash
    unsigned char hash3Check[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, RAp_a, PUF_SIZE);
    addToHash(ctx, NC_a, PUF_SIZE);
    addToHash(ctx, CA_c, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash3Check);

    res = memcmp(hash3, hash3Check, PUF_SIZE) == 0;
    std::cout << "Are the two hashes the same : " << res << "\n";

    if(res == 0){
        return 1;
    }

    std::cout << "The ACK hash is verified\n";

    return 0;
}

int main(){

    // Creation of the drones

    UAV A = UAV(idA);
    UAV B = UAV(idB);
    UAV C = UAV(idC);
    puf BS_PUF = puf();

    std::cout << "The first drone id is : " <<A.getId() << ".\n"; 
    std::cout << "The second drone id is : " <<B.getId() << ".\n"; 
    std::cout << "The third drone id is : " <<C.getId() << ".\n"; 

    int ret = enrolment(&A, &B);
    if (ret == 1){
        return ret;
    }
    
    ret = supplementaryEnrollment(&A, &C, &BS_PUF);
    if (ret == 1){
        return ret;
    }
    
    ret = supplementaryAutentication(&C, &A);
    if (ret == 1){
        return ret;
    }
    return 0;
}

// auto start = std::chrono::high_resolution_clock::now(); 
// auto end = std::chrono::high_resolution_clock::now();
// auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
// std::cout << "Execution time for A to enroll with B : " << duration.count() << " microseconds" << std::endl;