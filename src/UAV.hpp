/**
 * @file UAV.hpp
 * @brief UAV class header.
 * 
 * This file is the UAV class header. 
 * 
 */

#ifndef UAV_HPP
#define UAV_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <cstring>  // For memcpy

#include "utils.hpp"
#include "puf.hpp"
#include "SocketModule.hpp"

#define PUF_SIZE 32  // 256 bits = 32 bytes

/// @brief This class defines the data structure holded by UAVs' table to describe other UAVs.
class UAVData {
private:
    unsigned char* x;
    unsigned char* c;
    unsigned char* r;
    unsigned char* xLock;
    unsigned char* secret;

    void updatePointer(unsigned char*& ptr, const unsigned char* newData);

public:
    UAVData(
        const unsigned char* x = nullptr,
        const unsigned char* c = nullptr,
        const unsigned char* r = nullptr,
        const unsigned char* xLock = nullptr,
        const unsigned char* secret = nullptr
    );

    ~UAVData();
    UAVData(const UAVData& other);
    UAVData& operator=(const UAVData& other);

    const unsigned char* getX() const;
    const unsigned char* getC() const;
    const unsigned char* getR() const;
    const unsigned char* getXLock() const;
    const unsigned char* getSecret() const;

    void setX(const unsigned char* newX);
    void setC(const unsigned char* newC);
    void setR(const unsigned char* newR);
    void setXLock(const unsigned char* newXLock);
    void setSecret(const unsigned char* newSecret);

    void print() const;
};

/// @brief This class represents a UAV. It provides methods to manage its neighbours and access its PUF.
class UAV {
private:
    std::string id;
    std::unordered_map<std::string, UAVData> uavTable;
    const puf PUF;

public:
    SocketModule socketModule = SocketModule(); 
    UAV(std::string id);
    UAV(std::string id, unsigned char * salt);
    std::string getId();
    
    void addUAV(
        const std::string& id, 
        const unsigned char* x = nullptr,
        const unsigned char* c = nullptr,
        const unsigned char* r = nullptr,
        const unsigned char* xLock = nullptr,
        const unsigned char* secret = nullptr
    );
    
    bool removeUAV(const std::string& id);
    
    UAVData* getUAVData(const std::string& id);

    void callPUF(const unsigned char * input, unsigned char * response);

    // void printSalt(){
    //     PUF.printSalt();
    // }

};

#endif // UAV_HPP
