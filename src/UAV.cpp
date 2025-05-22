/**
 * @file UAV.cpp
 * @brief UAV class implementation.
 * 
 * This file implements the UAV class. 
 * 
 */
#include "UAV.hpp"

/// @brief Constructor
UAVData::UAVData(
    const unsigned char* x, const unsigned char* c, const unsigned char* r, 
    const unsigned char* xLock, const unsigned char* secret
) : x(nullptr), c(nullptr), r(nullptr), xLock(nullptr), secret(nullptr) {
    if (x) { this->x = new unsigned char[PUF_SIZE]; memcpy(this->x, x, PUF_SIZE); }
    if (c) { this->c = new unsigned char[PUF_SIZE]; memcpy(this->c, c, PUF_SIZE); }
    if (r) { this->r = new unsigned char[PUF_SIZE]; memcpy(this->r, r, PUF_SIZE); }
    if (xLock) { this->xLock = new unsigned char[PUF_SIZE]; memcpy(this->xLock, xLock, PUF_SIZE); }
    if (secret) { this->secret = new unsigned char[PUF_SIZE]; memcpy(this->secret, secret, PUF_SIZE); }
}

/// @brief Destructor
UAVData::~UAVData() {
    delete[] x;
    delete[] c;
    delete[] r;
    delete[] xLock;
    delete[] secret;
}

/// @brief Copy Constructor
UAVData::UAVData(const UAVData& other) : x(nullptr), c(nullptr), r(nullptr), xLock(nullptr), secret(nullptr) {
    if (other.x) { this->x = new unsigned char[PUF_SIZE]; memcpy(this->x, other.x, PUF_SIZE); }
    if (other.c) { this->c = new unsigned char[PUF_SIZE]; memcpy(this->c, other.c, PUF_SIZE); }
    if (other.r) { this->r = new unsigned char[PUF_SIZE]; memcpy(this->r, other.r, PUF_SIZE); }
    if (other.xLock) { this->xLock = new unsigned char[PUF_SIZE]; memcpy(this->xLock, other.xLock, PUF_SIZE); }
    if (other.secret) { this->secret = new unsigned char[PUF_SIZE]; memcpy(this->secret, other.secret, PUF_SIZE); }
}

/// @brief Copy Assignment Operator
UAVData& UAVData::operator=(const UAVData& other) {
    if (this == &other) return *this; // Avoid self-assignment

    delete[] x; delete[] c; delete[] r; delete[] xLock; delete[] secret;

    x = (other.x) ? new unsigned char[PUF_SIZE] : nullptr;
    c = (other.c) ? new unsigned char[PUF_SIZE] : nullptr;
    r = (other.r) ? new unsigned char[PUF_SIZE] : nullptr;
    xLock = (other.xLock) ? new unsigned char[PUF_SIZE] : nullptr;
    secret = (other.secret) ? new unsigned char[PUF_SIZE] : nullptr;

    if (x) memcpy(x, other.x, PUF_SIZE);
    if (c) memcpy(c, other.c, PUF_SIZE);
    if (r) memcpy(r, other.r, PUF_SIZE);
    if (xLock) memcpy(xLock, other.xLock, PUF_SIZE);
    if (secret) memcpy(secret, other.secret, PUF_SIZE);

    return *this;
}

/// @brief Getter Methods
const unsigned char* UAVData::getX() const { return x; }
const unsigned char* UAVData::getC() const { return c; }
const unsigned char* UAVData::getR() const { return r; }
const unsigned char* UAVData::getXLock() const { return xLock; }
const unsigned char* UAVData::getSecret() const { return secret; }

/// @brief Setter Methods
void UAVData::setX(const unsigned char* newX) { updatePointer(x, newX); }
void UAVData::setC(const unsigned char* newC) { updatePointer(c, newC); }
void UAVData::setR(const unsigned char* newR) { updatePointer(r, newR); }
void UAVData::setXLock(const unsigned char* newXLock) { updatePointer(xLock, newXLock); }
void UAVData::setSecret(const unsigned char* newSecret) { updatePointer(secret, newSecret); }

/// @brief Helper function to safely update pointers
void UAVData::updatePointer(unsigned char*& dest, const unsigned char* src) {
    delete[] dest;  // Free previous memory if allocated
    if (src) {
        dest = new unsigned char[PUF_SIZE];
        memcpy(dest, src, PUF_SIZE);
    } else {
        dest = nullptr;
    }
}

// UAV
/// @brief Constructor implementation
UAV::UAV(std::string id) : id(id), PUF() {}

UAV::UAV(std::string id, unsigned char * salt) : id(id), PUF(salt) {}

/// @brief Method implementation
std::string UAV::getId() {
    return this->id;
}

/// @brief Add an UAV to the UAV table.
/// @param id 
/// @param x 
/// @param c 
/// @param r 
/// @param xLock 
/// @param secret 
void UAV::addUAV(
        const std::string& id, 
        const unsigned char* x,
        const unsigned char* c,
        const unsigned char* r,
        const unsigned char* xLock,
        const unsigned char* secret
    ){
    uavTable[id] = UAVData(x,c,r,xLock,secret);
}

/// @brief Remove an UAV from the UAv table.
/// @param id 
/// @return 
bool UAV::removeUAV(const std::string& id) {
    return uavTable.erase(id) > 0;
}

/// @brief Get a UAV data in the UAV table.
/// @param id 
/// @return 
UAVData* UAV::getUAVData(const std::string& id) {
    auto it = uavTable.find(id);
    return (it != uavTable.end()) ? &(it->second) : nullptr;
}

/// @brief Call the UAV internal PUF for a computation.
/// @param input 
/// @param response 
void UAV::callPUF(const unsigned char * input, unsigned char * response){
    this->PUF.process(input, sizeof(input), response);
}

/// @brief Print the UAV data.
/// @param none
int UAV::enrolment_client(){
    std::cout << "\nEnrolment process begins.\n";

    // A enroll with B
    // A computes the challenge for B
    unsigned char xB[PUF_SIZE];
    generate_random_bytes(xB, PUF_SIZE);
    std::cout << "xB : "; print_hex(xB, PUF_SIZE);

    // Creates B in the memory of A and save xB 
    this->addUAV("B", xB);

    // Creates the challenge for B
    unsigned char CB[PUF_SIZE];
    this->callPUF(xB, CB);
    std::cout << "CB : "; print_hex(CB, PUF_SIZE);

    // Sends CB
    json msg = {{"id", this->getId()}, {"CB", toHexString(CB, PUF_SIZE)}};
    this->socketModule.sendMessage(msg);
    std::cout << "Sent CB.\n";

    // Wait for B's response (with RB)
    json rsp = this->socketModule.receiveMessage();
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

    this->getUAVData("B")->setR(RB);

    std::cout << "\nB is enroled to A\n";

    // B enroll with A
    // A receive CA. It saves CA.
    rsp = this->socketModule.receiveMessage();
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
    this->getUAVData("B")->setC(CA);

    // A computes RA
    unsigned char RA[PUF_SIZE];
    this->callPUF(CA, RA);
    std::cout << "RA : "; print_hex(RA, PUF_SIZE);

    // A sends RA
    msg = {{"id", this->getId()}, {"RA", toHexString(RA, PUF_SIZE)}};
    this->socketModule.sendMessage(msg);
    std::cout << "Sent RA.\n";

    return 0;
}

/// @brief Authenticate the UAV.
/// @param none
/// @return 0 if success, 1 if failure
int UAV::autentication_client(){
    // The client initiate the authentication process
    std::cout << "\nAutentication process begins.\n";

    // A generates a nonce NA 
    unsigned char NA[PUF_SIZE];
    generate_random_bytes(NA);
    std::cout << "NA : "; print_hex(NA, PUF_SIZE);

    const unsigned char * CA = this->getUAVData("B")->getC();
    if (CA == nullptr){
        std::cout << "No expected challenge in memory for this UAV.\n";
        return 1;
    }
    
    unsigned char M0[PUF_SIZE];
    xor_buffers(NA,CA,PUF_SIZE,M0);
    std::cout << "M0 : "; print_hex(M0, PUF_SIZE);

    // A sends its ID and NA to B 
    json msg = {{"id", this->getId()}, {"M0", toHexString(M0, PUF_SIZE)}};
    this->socketModule.sendMessage(msg);
    std::cout << "Sent ID and M0.\n";

    // A waits for the answer
    json rsp = this->socketModule.receiveMessage();
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
    this->callPUF(CA,RA);
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
        const unsigned char * xLock = this->getUAVData("B")->getXLock();
        if (xLock == nullptr){
            std::cout << "No old challenge in memory for the requested UAV.\n";
            return 1;
        }
        const unsigned char * secret = this->getUAVData("B")->getSecret();
        if (secret == nullptr){
            std::cout << "No old challenge in memory for the requested UAV.\n";
            return 1;
        }
        unsigned char lock[PUF_SIZE];
        this->callPUF(xLock, lock);
        unsigned char CAOld[PUF_SIZE]; 
        xor_buffers(lock, secret, PUF_SIZE, CAOld);

        // A will calculate the Nonce A that the server calculated with the wrong CA 
        unsigned char NAOld[PUF_SIZE];
        xor_buffers(M0, CAOld, PUF_SIZE, NAOld);
        std::cout << "NAOld : "; print_hex(NAOld, PUF_SIZE);

        // A will calculate the old response 
        unsigned char RAOld[PUF_SIZE];
        this->callPUF(CAOld, RAOld);
        std::cout << "RAOld : "; print_hex(RAOld, PUF_SIZE);

        // A will deduce NB from the old response
        unsigned char NBOld[PUF_SIZE];
        xor_buffers(M1, RAOld, PUF_SIZE, NBOld);
        xor_buffers(NBOld, NAOld, PUF_SIZE, NBOld);
        std::cout << "NBOld : "; print_hex(NBOld, PUF_SIZE);

        // A now tries to verify the hash with this value
        ctx = initHash();
        addToHash(ctx, CAOld, PUF_SIZE);
        addToHash(ctx, NBOld, PUF_SIZE);
        addToHash(ctx, RAOld, PUF_SIZE);
        addToHash(ctx, NAOld, PUF_SIZE);
        calculateHash(ctx, hash1Check);

        res = memcmp(hash1, hash1Check, PUF_SIZE) == 0;
        if (res == 0){
            std::cout << "Even with the old challenge, autentication has failed.\n";
            return 1;
        }
        std::cout << "B has been autenticated by A with the old challenge.\n";

        // A will now change the values to be the one obtained of the old challenge
        this->getUAVData("B")->setC(CAOld);
        memcpy(RA, RAOld, PUF_SIZE);
        memcpy(NB, NBOld, PUF_SIZE);
        memcpy(NA, NAOld, PUF_SIZE);
        memcpy(NA, NAOld, PUF_SIZE);

    }

    std::cout << "B's hash has been verified. B is autenticated to A.\n";

    // A will now conputes the new challenge and M2
    unsigned char RAp[PUF_SIZE];
    this->callPUF(NB,RAp);
    std::cout << "RAp : "; print_hex(RAp, PUF_SIZE);

    unsigned char M2[PUF_SIZE];
    xor_buffers(NA,RAp,PUF_SIZE,M2);
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
        {"id", this->getId()},
        {"M2", toHexString(M2, PUF_SIZE)},
        {"hash2", toHexString(hash2, PUF_SIZE)}
    };
    this->socketModule.sendMessage(msg);
    std::cout << "Sent ID, M2 and hash2.\n";

    // A waits for B's ACK
    rsp = this->socketModule.receiveMessage();
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
        this->callPUF(xLock, lock);
    
        unsigned char concealedCA[PUF_SIZE];
        xor_buffers(CA,lock,PUF_SIZE,concealedCA);
    
        this->getUAVData("B")->setXLock(xLock);
        this->getUAVData("B")->setSecret(concealedCA);

        // Then A saves the new challenge in CA
        this->getUAVData("B")->setC(NB);

        return 1;
    }

    // Then A saves the new challenge in CA
    this->getUAVData("B")->setC(NB);

    // Finished
    std::cout << "\nThe two UAV autenticated each other.\n";
    
    return 0;
}

/// @brief Enrolment of the UAV.
/// @param none
/// @return 0 if success, 1 if failure

int UAV::enrolment_server(){
    std::cout << "\nEnrolment process begins.\n";

    // B waits for B's message  (with CB)
    json rsp = this->socketModule.receiveMessage();
    printJSON(rsp);

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
    this->addUAV("A", nullptr, CB);

    // B computes RB
    unsigned char RB[PUF_SIZE];
    this->callPUF(CB, RB);
    std::cout << "RB : "; print_hex(RB, PUF_SIZE);

    // B sends RB
    json msg = {{"id", this->getId()}, {"RB", toHexString(RB, PUF_SIZE)}};
    this->socketModule.sendMessage(msg);
    std::cout << "Sent RB.\n";

    // B enroll with A
    // Computes xA
    unsigned char xA[PUF_SIZE];
    generate_random_bytes(xA, PUF_SIZE);
    std::cout << "xA : "; print_hex(xA, PUF_SIZE);

    // Save xA
    this->getUAVData("A")->setX(xA);

    // Creates the challenge for A
    unsigned char CA[PUF_SIZE];
    this->callPUF(xA, CA);
    std::cout << "CA : "; print_hex(CA, PUF_SIZE);

    // Sends CA
    msg = {{"id", this->getId()}, {"CA", toHexString(CA, PUF_SIZE)}};
    this->socketModule.sendMessage(msg);
    std::cout << "Sent CA.\n";

    // B receive RA and saves it. 
    rsp = this->socketModule.receiveMessage();
    printJSON(rsp);

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
    this->getUAVData("A")->setR(RA);
    
    return 0;
}

/// @brief Authenticate the UAV.
/// @param none
/// @return 0 if success, 1 if failure
int UAV::autentication_server(){
    // The client initiate the autentication process
    std::cout << "\nAutentication process begins.\n";

    // B receive the initial message
    json rsp = this->socketModule.receiveMessage();
    printJSON(rsp);

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
    const unsigned char * xA = this->getUAVData("A")->getX();
    if (xA == nullptr){
        std::cout << "No challenge in memory for the requested UAV.\n";
        return 1;
    }
    std::cout << "xA : "; print_hex(xA, PUF_SIZE);
    
    unsigned char CA[PUF_SIZE];
    this->callPUF(xA,CA);
    std::cout << "CA : "; print_hex(CA, PUF_SIZE);
    
    unsigned char NA[PUF_SIZE];
    xor_buffers(M0, CA, PUF_SIZE, NA);
    std::cout << "NA : "; print_hex(NA, PUF_SIZE);

    // B then creates a nonce NB and the secret message M1 
    unsigned char gammaB[PUF_SIZE];
    unsigned char NB[PUF_SIZE];
    generate_random_bytes(gammaB);
    this->callPUF(gammaB,NB);
    std::cout << "NB : "; print_hex(NB, PUF_SIZE);

    unsigned char M1[PUF_SIZE];
    const unsigned char * RA = this->getUAVData("A")->getR();
    if (RA == nullptr){
        std::cout << "No response in memory for the requested UAV.\n";
        return 1;
    }
    std::cout << "RA : "; print_hex(RA, PUF_SIZE);
    xor_buffers(RA,NA,PUF_SIZE,M1);
    xor_buffers(M1,NB,PUF_SIZE,M1);
    std::cout << "M1 : "; print_hex(M1, PUF_SIZE);

    // B sends its ID, M1 and a hash of CA, NB, RA, NA to A
    unsigned char hash1[PUF_SIZE];
    EVP_MD_CTX * ctx = initHash();
    addToHash(ctx, CA, PUF_SIZE);
    addToHash(ctx, NB, PUF_SIZE);
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash1);
    std::cout << "hash1 : "; print_hex(hash1, PUF_SIZE);
    
    json msg = {
        {"id", this->getId()}, 
        {"M1", toHexString(M1, PUF_SIZE)}, 
        {"hash1", toHexString(hash1, PUF_SIZE)}
    };
    this->socketModule.sendMessage(msg);
    std::cout << "Sent ID, M1 and hash1.\n";

    // B waits for A response (M2)
    rsp = this->socketModule.receiveMessage();
    printJSON(rsp);

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
    xor_buffers(M2, NA, PUF_SIZE, RAp);
    std::cout << "RAp : "; print_hex(RAp, PUF_SIZE);

    // B verify the hash
    unsigned char hash2Check[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, NB, PUF_SIZE);
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx, RAp, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash2Check);
    std::cout << "hash2Check : "; print_hex(hash2Check, PUF_SIZE);

    int res = memcmp(hash2, hash2Check, PUF_SIZE) == 0;
    std::cout << "B verify A's hash : " << res << "\n";

    if(res == 0){
        std::cout << "The hashes do not correspond.\n";
        return 1;
    }

    std::cout << "A's hash has been verified. A is autenticated to B.";

    // B changes its values
    this->getUAVData("A")->setX(gammaB);
    this->getUAVData("A")->setR(RAp);

    // B sends a hash of RAp, NB, NA as an ACK
    unsigned char hash3[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, RAp, PUF_SIZE);
    addToHash(ctx, NB, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash3);
    std::cout << "hash3 : "; print_hex(hash3, PUF_SIZE);
    
    msg = {{"id", this->getId()}, {"hash3", toHexString(hash3, PUF_SIZE)}};
    this->socketModule.sendMessage(msg);
    std::cout << "Sent ID and hash3.\n";

    // Finished
    std::cout << "\nThe two UAV autenticated each other.\n";

    return 0;
}


/// @brief Pre-enrolment function for initialize the authentication of UAV A
/// @param none
/// @return 0 if succeded, 1 if failed
int UAV::preEnrolment(){

    // A waits for BS's query
    json rsp = this->socketModule.receiveMessage();
    printJSON(rsp);

    // Check if an error occurred
    if (rsp.contains("error")) {
        std::cerr << "Error occurred: " << rsp["error"] << std::endl;
        return -1;
    }

    if(!rsp.contains("data")){
        std::cerr << "Error occurred: no member data" << std::endl;
        return 1;
    }
    std::vector<std::string> receivedHexList = rsp["data"];

    // Convert each hex string back to unsigned char arrays
    unsigned char receivedArrays[CHALLENGE_SIZE][PUF_SIZE];

    for (size_t i = 0; i < receivedHexList.size() && i < receivedHexList.size(); i++) {
        fromHexString(receivedHexList[i], receivedArrays[i], PUF_SIZE);
    }

    // Generates the responses
    unsigned char LR[CHALLENGE_SIZE][PUF_SIZE];
    for (int i = 0; i < CHALLENGE_SIZE && i < CHALLENGE_SIZE; i++) {
        this->callPUF(receivedArrays[i], LR[i]);
        std::cout << "LR[" << i << "]: "; print_hex(LR[i], PUF_SIZE);
    }

    // Send the responses back
    json msg = {{"id", this->getId()}};
    json dataArray = json::array();

    for (int i = 0; i < CHALLENGE_SIZE; i++) {
        dataArray.push_back(toHexString(LR[i], 32));
    }   
    msg["data"] = dataArray;
    this->socketModule.sendMessage(msg);

    return 0;
}

int UAV::preEnrolmentRetrival(){

    std::cout << "\nC will now retrieve A's credentials.\n";

    // Wait for A's credentials
    json rsp = this->socketModule.receiveMessage();
    printJSON(rsp);

    // Check if an error occurred
    if (rsp.contains("error")) {
        std::cerr << "Error occurred: " << rsp["error"] << std::endl;
        return -1;
    }

    // Retrieve CA and RA from the msg
    unsigned char CA[PUF_SIZE];
    if(!rsp.contains("CA")){
        std::cerr << "Error occurred: no member CA" << std::endl;
        return 1;
    }
    fromHexString(rsp["CA"].get<std::string>(), CA, PUF_SIZE);
    unsigned char RA[PUF_SIZE];
    if(!rsp.contains("RA")){
        std::cerr << "Error occurred: no member RA" << std::endl;
        return 1;
    }
    fromHexString(rsp["RA"].get<std::string>(), RA, PUF_SIZE);
    std::cout << "CA : "; print_hex(CA, PUF_SIZE);
    std::cout << "RA : "; print_hex(RA, PUF_SIZE);

    // Conceals RA
    unsigned char xLock[PUF_SIZE];
    generate_random_bytes(xLock);
    std::cout << "xLock : "; print_hex(xLock, PUF_SIZE);

    unsigned char lock[PUF_SIZE];
    this->callPUF(xLock, lock);

    unsigned char secret[PUF_SIZE];
    xor_buffers(RA, lock, PUF_SIZE, secret);
    std::cout << "secret : "; print_hex(secret, PUF_SIZE);


    this->addUAV("A", nullptr, CA, nullptr, xLock, secret);
    std::cout << "\nC has retrieved A's credentials.\n";

    return 0;
}

/// @brief If the PreEnrolment doesnt fail, this comes to help
/// @param none
/// @return 0 if succeded, 1 if failed
int UAV::supplementaryAuthenticationInitial(){

    // Waits for C demands
    json rsp = this->socketModule.receiveMessage();
    printJSON(rsp);

    // Check if an error occurred
    if (rsp.contains("error")) {
        std::cerr << "Error occurred: " << rsp["error"] << std::endl;
        return -1;
    }

    // A retrieve the id of the UAV trying to connect
    std::string idC = rsp["id"].get<std::string>();
    
    //  and search it's table for a corresponding UAV 
    UAVData* data = this->getUAVData(idC);
    if (data) {
        std::cout << "UAVData found! Not supposed to happen ?! Quit.\n" << std::endl;
        return 1;
    } 
    std::cout << "UAVData not found for idC.\n" << std::endl;
    std::cout << "Initiate supplementary authentication\n" << std::endl;
    
    // A generates an nonce NA
    unsigned char NA[PUF_SIZE];
    generate_random_bytes(NA);
    std::cout << "NA : "; print_hex(NA, PUF_SIZE);

    json msg = {{"id", "A"}, {"NA", toHexString(NA, PUF_SIZE)}};

    // A sends 
    this->socketModule.sendMessage(msg);
    std::cout << "Sent ID and NA.\n";

    // Waits for C's response 
    rsp = this->socketModule.receiveMessage();
    printJSON(rsp);

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
    this->callPUF(CA, RA);
    std::cout << "RA : "; print_hex(RA, PUF_SIZE);
    
    // A retrieve NC from M1 
    unsigned char NC[PUF_SIZE];
    xor_buffers(M1, RA, PUF_SIZE, NC);
    std::cout << "NC : "; print_hex(NC, PUF_SIZE);

    // A verify the hash
    unsigned char hash1Check[PUF_SIZE];
    EVP_MD_CTX * ctx = initHash();
    addToHash(ctx, idC);
    addToHash(ctx, CA, PUF_SIZE);
    addToHash(ctx, NC, PUF_SIZE);
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash1Check);
    std::cout << "hash1Check : "; print_hex(hash1Check, PUF_SIZE);

    bool res = memcmp(hash1, hash1Check, PUF_SIZE) == 0;
    std::cout << "A verify C's hash : " << res << "\n";

    if(res == 0){
        std::cout << "The autentication failed.\n";
        return res;

    }

    std::cout << "C's hash has been verified. C is autenticated to A.\n";

    // A will now conputes the new challenge and M2
    unsigned char RAp[PUF_SIZE];
    this->callPUF(NC,RAp);
    std::cout << "RAp : "; print_hex(RAp, PUF_SIZE);

    unsigned char M2[PUF_SIZE];
    xor_buffers(NC,RAp,PUF_SIZE,M2);
    std::cout << "M2 : "; print_hex(M2, PUF_SIZE);

    // A sends M2, and a hash of NB, RA, RAp, NA
    unsigned char hash2[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, NC, PUF_SIZE);
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx, RAp, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash2);
    std::cout << "hash2 : "; print_hex(hash2, PUF_SIZE);

    // TODO : send M2, and a hash of NB, RA, RAp, NA
    msg = {
        {"id", this->getId()},
        {"M2", toHexString(M2, PUF_SIZE)},
        {"hash2", toHexString(hash2, PUF_SIZE)}
    };
    this->socketModule.sendMessage(msg);
    std::cout << "Sent ID, M2 and hash2.\n";

    // A waits for C's ACK
    rsp = this->socketModule.receiveMessage();
    printJSON(rsp);

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
        this->callPUF(xLock, lock);
    
        unsigned char concealedCA[PUF_SIZE];
        xor_buffers(CA,lock,PUF_SIZE,concealedCA);
    
        this->getUAVData(idC)->setXLock(xLock);
        this->getUAVData(idC)->setSecret(concealedCA);

        // Then A saves the new challenge in CA
        this->addUAV(idC, nullptr, NC);

        return 1;
    }

    // Then A saves the new challenge in CA
    this->addUAV(idC, nullptr, NC);

    // Finished
    std::cout << "\nThe two UAV autenticated each other.\n";

    return 0;
}

int UAV::supplementaryAuthenticationSup(){

    // C will now try to connect to A
    json msg = {{"id", this->getId()}};
    this->socketModule.sendMessage(msg);
    std::cout << "Sent ID.\n";

    // Wait for answer
    json rsp = this->socketModule.receiveMessage();
    printJSON(rsp);

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
    const unsigned char * CA = this->getUAVData("A")->getC();
    if (CA == nullptr){
        std::cout << "No challenge in memory for the requested UAV.\n";
        return 1;
    }
    std::cout << "CA : "; print_hex(CA, PUF_SIZE);

    const unsigned char * xLock = this->getUAVData("A")->getXLock();
    std::cout << "xLock : "; print_hex(xLock, PUF_SIZE);
    const unsigned char * secret = this->getUAVData("A")->getSecret();
    std::cout << "secret : "; print_hex(secret, PUF_SIZE);
    unsigned char lock[PUF_SIZE];
    this->callPUF(xLock,lock);
    unsigned char RA[PUF_SIZE];
    xor_buffers(lock, secret, PUF_SIZE, RA);
    std::cout << "RA : "; print_hex(RA, PUF_SIZE);

    // C then creates a nonce NC and the secret message M1 
    unsigned char gammaC[PUF_SIZE];
    unsigned char NC[PUF_SIZE];
    generate_random_bytes(gammaC);
    this->callPUF(gammaC,NC);
    std::cout << "NC : "; print_hex(NC, PUF_SIZE);

    unsigned char M1[PUF_SIZE];

    xor_buffers(RA,NC,PUF_SIZE,M1);
    std::cout << "M1 : "; print_hex(M1, PUF_SIZE);

    // C sends its ID, M1 and a hash of idC, CA, NB, RA, NA to A
    unsigned char hash1[PUF_SIZE];
    EVP_MD_CTX * ctx = initHash();
    addToHash(ctx, this->getId());
    addToHash(ctx, CA, PUF_SIZE);
    addToHash(ctx, NC, PUF_SIZE);
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash1);
    std::cout << "hash1 : "; print_hex(hash1, PUF_SIZE);
    
    msg = {
        {"id", this->getId()}, 
        {"CA", toHexString(CA, PUF_SIZE)}, 
        {"M1", toHexString(M1, PUF_SIZE)}, 
        {"hash1", toHexString(hash1, PUF_SIZE)}
    };
    this->socketModule.sendMessage(msg);
    std::cout << "Sent ID, CA, M1 and hash1.\n";

    // Wait for A's response 
    rsp = this->socketModule.receiveMessage();
    printJSON(rsp);

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
    this->getUAVData("A")->setX(gammaC);
    this->getUAVData("A")->setR(RAp);

    // B sends a hash of RAp, NB, NA as an ACK
    unsigned char hash3[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, RAp, PUF_SIZE);
    addToHash(ctx, NC, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash3);
    std::cout << "hash3 : "; print_hex(hash3, PUF_SIZE);
    
    msg = {{"id", this->getId()}, {"hash3", toHexString(hash3, PUF_SIZE)}};
    this->socketModule.sendMessage(msg);
    std::cout << "Sent ID and hash3.\n";

    // Finished
    std::cout << "\nThe two UAV autenticated each other.\n";

    return 0;
}

/// @brief The autentication from UAV A failed
/// @param none 
/// @return 0 if succeded, 1 if failed
int UAV::failed_autentication_client(){
    // The client initiate the authentication process
    std::cout << "\nAutentication process begins.\n";

    // A generates a nonce NA 
    unsigned char NA[PUF_SIZE];
    generate_random_bytes(NA);
    std::cout << "NA : "; print_hex(NA, PUF_SIZE);

    const unsigned char * CA = this->getUAVData("B")->getC();
    if (CA == nullptr){
        std::cout << "No expected challenge in memory for this UAV.\n";
        return 1;
    }
    
    unsigned char M0[PUF_SIZE];
    xor_buffers(NA,CA,PUF_SIZE,M0);
    std::cout << "M0 : "; print_hex(M0, PUF_SIZE);

    // A sends its ID and NA to B 
    json msg = {{"id", this->getId()}, {"M0", toHexString(M0, PUF_SIZE)}};
    this->socketModule.sendMessage(msg);
    std::cout << "Sent ID and M0.\n";

    // A waits for the answer
    json rsp = this->socketModule.receiveMessage();
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
    unsigned char RA[PUF_SIZE];
    this->callPUF(CA,RA);
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
        const unsigned char * xLock = this->getUAVData("B")->getXLock();
        if (xLock == nullptr){
            std::cout << "No old challenge in memory for the requested UAV.\n";
            return 1;
        }
        const unsigned char * secret = this->getUAVData("B")->getSecret();
        if (secret == nullptr){
            std::cout << "No old challenge in memory for the requested UAV.\n";
            return 1;
        }
        unsigned char lock[PUF_SIZE];
        this->callPUF(xLock, lock);
        unsigned char CAOld[PUF_SIZE]; 
        xor_buffers(lock, secret, PUF_SIZE, CAOld);

        // A will calculate the Nonce A that the server calculated with the wrong CA 
        unsigned char NAOld[PUF_SIZE];
        xor_buffers(M0, CAOld, PUF_SIZE, NAOld);
        std::cout << "NAOld : "; print_hex(NAOld, PUF_SIZE);

        // A will calculate the old response 
        unsigned char RAOld[PUF_SIZE];
        this->callPUF(CAOld, RAOld);
        std::cout << "RAOld : "; print_hex(RAOld, PUF_SIZE);

        // A will deduce NB from the old response
        unsigned char NBOld[PUF_SIZE];
        xor_buffers(M1, RAOld, PUF_SIZE, NBOld);
        xor_buffers(NBOld, NAOld, PUF_SIZE, NBOld);
        std::cout << "NBOld : "; print_hex(NBOld, PUF_SIZE);

        // A now tries to verify the hash with this value
        ctx = initHash();
        addToHash(ctx, CAOld, PUF_SIZE);
        addToHash(ctx, NBOld, PUF_SIZE);
        addToHash(ctx, RAOld, PUF_SIZE);
        addToHash(ctx, NAOld, PUF_SIZE);
        calculateHash(ctx, hash1Check);

        res = memcmp(hash1, hash1Check, PUF_SIZE) == 0;
        if (res == 0){
            std::cout << "Even with the old challenge, autentication has failed.\n";
            return 1;
        }
        std::cout << "B has been autenticated by A with the old challenge.\n";

        // A will now change the values to be the one obtained of the old challenge
        this->getUAVData("B")->setC(CAOld);
        memcpy(RA, RAOld, PUF_SIZE);
        memcpy(NB, NBOld, PUF_SIZE);
        memcpy(NA, NAOld, PUF_SIZE);

    }

    std::cout << "B's hash has been verified. B is autenticated to A.\n";

    // A will now conputes the new challenge and M2
    unsigned char RAp[PUF_SIZE];
    this->callPUF(NB,RAp);
    std::cout << "RAp : "; print_hex(RAp, PUF_SIZE);

    unsigned char M2[PUF_SIZE];
    xor_buffers(NA,RAp,PUF_SIZE,M2);
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

    // TODO : send M2, and a hash of NB, RA, RAp, NA
    msg = {
        {"id", this->getId()},
        {"M2", toHexString(M2, PUF_SIZE)},
        {"hash2", toHexString(hash2, PUF_SIZE)}
    };

    // ##################### MODIFICATION TO FAIL #####################
    
    // The message is not sent to B
    
    // A->socketModule.sendMessage(msg);
    // std::cout << "Sent ID, M2 and hash2.\n";
    
    // ################################################################

    // A waits for B's ACK
    rsp = this->socketModule.receiveMessage();
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
        this->callPUF(xLock, lock);
    
        unsigned char concealedCA[PUF_SIZE];
        xor_buffers(CA,lock,PUF_SIZE,concealedCA);
    
        this->getUAVData("B")->setXLock(xLock);
        this->getUAVData("B")->setSecret(concealedCA);
        
        // Then A saves the new challenge in CA
        this->getUAVData("B")->setC(NB);
        return 1;
    }
    
    // Then A saves the new challenge in CA
    this->getUAVData("B")->setC(NB);

    // Finished

    return 0;
}