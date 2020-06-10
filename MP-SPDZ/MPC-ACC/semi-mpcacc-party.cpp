/*
 * semi-mpcacc-party.cpp
 *
 */

#include "GC/SemiSecret.h"
#include "GC/SemiPrep.h"

#include "Protocols/SemiMC.hpp"
#include "Protocols/SemiPrep.hpp"
#include "Protocols/SemiInput.hpp"
#include "Protocols/MAC_Check_Base.hpp"
#include "ot-mpcacc-party.h"

#include <assert.h>

int main(int argc, const char** argv)
{
    run<SemiShare>(argc, argv);
}
