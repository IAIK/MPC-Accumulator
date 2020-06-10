/*
 * mascot-mpcacc-party.cpp
 *
 */

#include "GC/TinierSecret.h"
#include "GC/TinyMC.h"

#include "Protocols/Share.hpp"
#include "Protocols/MAC_Check.hpp"
#include "GC/Secret.hpp"
#include "GC/TinierSharePrep.hpp"
#include "ot-mpcacc-party.h"

#include <assert.h>

int main(int argc, const char** argv)
{
    run<Share>(argc, argv);
}
