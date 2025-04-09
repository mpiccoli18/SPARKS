/**
 * @file test.cpp
 * @brief This file contains the programmers test of the different functionalities.
 */

#include "UAV.hpp"
#include "puf.hpp"
#include "utils.hpp"

int main() {
    
    UAV A = UAV("A");
    UAV B = UAV("B");

    std::cout << "The first drone is " <<A.getId() << ".\n"; 
    std::cout << "The second drone is " <<B.getId() << ".\n"; 

    // std::cout << "Let's enroll the drones with each others"; 
    
    std::cout << "Let's verify that the PUF are different in the two drones\n";

    unsigned char string1[32] = "When in disgrace with Fortune a";
    unsigned char string2[32] = "Haply I think on thee, then my ";

    unsigned char response1[32];
    unsigned char response2[32];

    A.callPUF(string1,response1);
    A.callPUF(string2,response2);

    // std::cout << "Salt of A is : ";
    // A.printSalt();
    // std::cout << "Salt of B is : ";
    // B.printSalt();

    unsigned char response3[32];
    unsigned char response4[32];

    B.callPUF(string1,response3);
    B.callPUF(string2,response4);

    // std::cout << "Salt of A is : ";
    // A.printSalt();
    // std::cout << "Salt of B is : ";
    // B.printSalt();

    std::cout << "For string1 : " << string1 << "\n"; 
    std::cout << "A responds (hex) : ";
    print_hex(response1, sizeof(response1));
    std::cout << "B responds (hex) : ";
    print_hex(response3, sizeof(response3));

    std::cout << "For string2 : " << string2 << "\n"; 
    std::cout << "A responds (hex) : ";
    print_hex(response2, sizeof(response2));
    std::cout << "B responds (hex) : ";
    print_hex(response4, sizeof(response4));
    

    // std::cout << "Salt of A is : ";
    // A.printSalt();
    // std::cout << "Salt of B is : ";
    // B.printSalt();

    std::cout << "\nLet's verify that the PUF are constant\n";


    std::cout << "Those four values : \n";
    print_hex(response1, sizeof(response1));
    print_hex(response2, sizeof(response2));
    print_hex(response3, sizeof(response3));
    print_hex(response4, sizeof(response4));

    A.callPUF(string1,response4);
    A.callPUF(string2,response3);
    B.callPUF(string1,response2);
    B.callPUF(string2,response1);

    std::cout << "Should be the same than those : \n";
    print_hex(response4, sizeof(response4));
    print_hex(response3, sizeof(response3));
    print_hex(response2, sizeof(response2));
    print_hex(response1, sizeof(response1));

    std::cout << "\nLet's enroll the drones with each others \n";

    // Get the values
    std::string idA = A.getId(); 
    std::string idB = B.getId(); 

    // Computes and save xB
    unsigned char xB[PUF_SIZE];
    generate_random_bytes(xB, PUF_SIZE);
    A.addUAV(idB, xB);


    // Computes and save xA
    unsigned char xA[PUF_SIZE];
    generate_random_bytes(xA, PUF_SIZE);
    B.addUAV(idA, xA);

    // Print
    std::cout << "Lets print the Xs : \n";
    std::cout << "xB :"; 
    print_hex(xB, sizeof(xB));
    std::cout << "xA :"; 
    print_hex(xA, sizeof(xA));

    // Computes CB and CA
    unsigned char CB[PUF_SIZE];
    A.callPUF(xB, CB);
    unsigned char CA[PUF_SIZE];
    B.callPUF(xA, CA);

    // Send CA and CB
    A.getUAVData(idB)->setC(CA);
    B.getUAVData(idA)->setC(CB);

    // Print
    std::cout << "Lets print the Cs : \n";
    std::cout << "CB :"; 
    print_hex(CB, sizeof(CB));
    std::cout << "CA :"; 
    print_hex(CA, sizeof(CA));

    // Computes RA and RB 
    unsigned char RA[PUF_SIZE];
    A.callPUF(CA, RA);
    unsigned char RB[PUF_SIZE];
    B.callPUF(CB, RB);

    // Send RA and RB
    A.getUAVData(idB)->setR(RB);
    B.getUAVData(idA)->setR(RA);

    // Print
    std::cout << "Lets print the Rs : \n";
    std::cout << "RA :"; 
    print_hex(RA, sizeof(RA));
    std::cout << "RB :"; 
    print_hex(RB, sizeof(RB));

    std::cout << "\nThe drones are now enrolled with each other.\nLet's prove that they can predict the answer of the other Drone to a challenge : \n";

    // Computes A.callPUF(CA)
    unsigned char recalcRA[PUF_SIZE]; 
    A.callPUF(A.getUAVData(idB)->getC(),recalcRA);

    // Gets saved RA
    const unsigned char * savedRA = B.getUAVData(idA)->getR(); 

    std::cout << "A.callPUF(CA) == B.getUAVData(idA)->getR() : " << (memcmp(recalcRA, savedRA, PUF_SIZE) == 0) << " \n";

    // Computes B.callPUF(CB)
    unsigned char recalcRB[PUF_SIZE]; 
    B.callPUF(B.getUAVData(idA)->getC(),recalcRB);

    // Gets saved RB
    const unsigned char * savedRB = A.getUAVData(idB)->getR(); 

    std::cout << "B.callPUF(CB) == A.getUAVData(idB)->getR() : " << (memcmp(recalcRB, savedRB, PUF_SIZE) == 0) << " \n";

    // Hash testing

    std::cout << "\nNow we test the hash function of the program :\n";

    unsigned char hash[32];

    std::cout << "First with different value types";
    EVP_MD_CTX* ctx = initHash();

    int num = 42;
    std::string text = "Hello";
    unsigned char buffer[] = {0x12, 0x34, 0x56};

    addToHash(ctx, num); 
    addToHash(ctx, text);
    addToHash(ctx, static_cast<const unsigned char*>(buffer), sizeof(buffer));

    calculateHash(ctx, hash);

    // Print hash
    print_hex(hash,PUF_SIZE);

    // Hash values from the drones 
    std::cout << "\nNow with real values from the drones : ";

    ctx = initHash();
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx,idA);
    addToHash(ctx,CB);

    calculateHash(ctx, hash);

    print_hex(hash, PUF_SIZE);

    std::cout << "\nWhat happens if i chang ethe places of the values ? : ";

    ctx = initHash();
    addToHash(ctx,CB);
    addToHash(ctx,idA);
    addToHash(ctx, RA, PUF_SIZE);

    calculateHash(ctx, hash);

    print_hex(hash, PUF_SIZE);

    std::cout << "\nWhat if I put them back in order ? ";

    ctx = initHash();
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx,idA);
    addToHash(ctx,CB);

    calculateHash(ctx, hash);

    print_hex(hash, PUF_SIZE);

    return 0;
} // c && g++ -o test test.cpp UAV.cpp puf.cpp utils.cpp -lcrypto -O0 && ./test