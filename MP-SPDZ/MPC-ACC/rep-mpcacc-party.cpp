/*
 * rep-mpcacc-party.cpp
 *
 */
#include "Protocols/Rep3Share.h"

#include "hm-mpcacc-party.h"

#include <assert.h>

template<>
Preprocessing<Rep3Share<gfp100>>* Preprocessing<Rep3Share<gfp100>>::get_live_prep(
        SubProcessor<Rep3Share<gfp100>>* proc, DataPositions& usage)
{
    return new ReplicatedPrep<Rep3Share<gfp100>>(proc, usage);
}

int main(int argc, const char** argv)
{
    run<Rep3Share>(argc, argv);
}
