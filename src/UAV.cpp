#include "UAV.hpp"

// Constructor
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

// Destructor
UAVData::~UAVData() {
    delete[] x;
    delete[] c;
    delete[] r;
    delete[] xLock;
    delete[] secret;
}

// Copy Constructor
UAVData::UAVData(const UAVData& other) : x(nullptr), c(nullptr), r(nullptr), xLock(nullptr), secret(nullptr) {
    if (other.x) { this->x = new unsigned char[PUF_SIZE]; memcpy(this->x, other.x, PUF_SIZE); }
    if (other.c) { this->c = new unsigned char[PUF_SIZE]; memcpy(this->c, other.c, PUF_SIZE); }
    if (other.r) { this->r = new unsigned char[PUF_SIZE]; memcpy(this->r, other.r, PUF_SIZE); }
    if (other.xLock) { this->xLock = new unsigned char[PUF_SIZE]; memcpy(this->xLock, other.xLock, PUF_SIZE); }
    if (other.secret) { this->secret = new unsigned char[PUF_SIZE]; memcpy(this->secret, other.secret, PUF_SIZE); }
}

// Copy Assignment Operator
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

// Getter Methods
const unsigned char* UAVData::getX() const { return x; }
const unsigned char* UAVData::getC() const { return c; }
const unsigned char* UAVData::getR() const { return r; }
const unsigned char* UAVData::getXLock() const { return xLock; }
const unsigned char* UAVData::getSecret() const { return secret; }

// Setter Methods
void UAVData::setX(const unsigned char* newX) { updatePointer(x, newX); }
void UAVData::setC(const unsigned char* newC) { updatePointer(c, newC); }
void UAVData::setR(const unsigned char* newR) { updatePointer(r, newR); }
void UAVData::setXLock(const unsigned char* newXLock) { updatePointer(xLock, newXLock); }
void UAVData::setSecret(const unsigned char* newSecret) { updatePointer(secret, newSecret); }

// Helper function to safely update pointers
void UAVData::updatePointer(unsigned char*& dest, const unsigned char* src) {
    delete[] dest;  // Free previous memory if allocated
    if (src) {
        dest = new unsigned char[PUF_SIZE];
        memcpy(dest, src, PUF_SIZE);
    } else {
        dest = nullptr;
    }
}

// Print function
void UAVData::print() const {
    std::cout << "UAVData:\n";
    if (x) { std::cout << "X: "; print_hex(x, PUF_SIZE); }
    if (c) { std::cout << "C: "; print_hex(c, PUF_SIZE); }
    if (r) { std::cout << "R: "; print_hex(r, PUF_SIZE); }
    if (xLock) { std::cout << "XLock: "; print_hex(xLock, PUF_SIZE); }
    if (secret) { std::cout << "Secret: "; print_hex(secret, PUF_SIZE); }
}

// UAV
// Constructor implementation
UAV::UAV(std::string id) : id(id), PUF() {}

// Method implementation
std::string UAV::getId() {
    return this->id;
}

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

bool UAV::removeUAV(const std::string& id) {
    return uavTable.erase(id) > 0;
}

UAVData* UAV::getUAVData(const std::string& id) {
    auto it = uavTable.find(id);
    return (it != uavTable.end()) ? &(it->second) : nullptr;
}

void UAV::callPUF(const unsigned char * input, unsigned char * response){
    this->PUF.process(input, sizeof(input), response);
}

