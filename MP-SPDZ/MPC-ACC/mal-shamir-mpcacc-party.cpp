/*
 * mal-shamir-mpcacc-party.cpp
 *
 */

#include "Protocols/MaliciousShamirShare.h"

#include "Protocols/Shamir.hpp"
#include "Protocols/ShamirInput.hpp"
#include "Protocols/ShamirMC.hpp"
#include "Protocols/MaliciousShamirMC.hpp"

#include "hm-mpcacc-party.h"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    ShamirOptions::singleton = {opt, argc, argv};
    run<MaliciousShamirShare>(argc, argv);
}
