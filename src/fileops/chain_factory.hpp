#ifndef CHAIN_FACTORY_HPP
#define CHAIN_FACTORY_HPP

#include <bitset>

#include <fileops/httpiochain.hpp>
#include <fileops/fileutils.hpp>

namespace Davix{

const int CHAIN_POSIX = 1;

typedef std::bitset<32> CreationFlags;

class ChainFactory
{
public:
    static HttpIOChain& instanceChain(const CreationFlags & flags, HttpIOChain & c);
private:
    ChainFactory();
};


}

#endif // CHAIN_FACTORY_HPP
