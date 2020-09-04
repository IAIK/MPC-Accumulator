/*
 * shamir-mpcacc-party.cpp
 *
 */
#include "Protocols/ShamirShare.h"

#include "Protocols/Shamir.hpp"
#include "Protocols/ShamirInput.hpp"
#include "Protocols/ShamirMC.hpp"
#include "hm-mpcacc-party.h"

#include <assert.h>

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    ShamirOptions::singleton = {opt, argc, argv};
    run<ShamirShare>(argc, argv);
}
