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

