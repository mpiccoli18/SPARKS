#ifndef PUF_HPP
#define PUF_HPP

#include <openssl/evp.h>

#include "utils.hpp"
#include "puf.hpp"

class puf
{
private:
    const unsigned char salt[PUF_SIZE];
public:
    puf();

    void process(const unsigned char * input, size_t size, unsigned char * output) const;
    // TODO : remove
    void printSalt() const{
        print_hex(salt,PUF_SIZE);
    }

};

#endif