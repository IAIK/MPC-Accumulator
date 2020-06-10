/*
 * semi-mpcacc-party.cpp
 *
 */

#include "GC/SemiSecret.h"
#include "GC/SemiPrep.h"

#include "Protocols/SemiMC.hpp"
#include "Protocols/SemiPrep.hpp"
#include "Protocols/SemiInput.hpp"

#include "ot-mpcacc-party.h"

#include <assert.h>

// README:
// In ReplicatedPrep.hpp:
//    void BufferPrep<T>::get_three_no_count(Dtype dtype, T& a, T& b, T& c)
//    void BufferPrep<T>::get_two_no_count(Dtype dtype, T& a, T& b)
// remove the pop_back function and call mpc_acc.preprocessing(3)

// In OTTripleSetup.cpp:
//    void OTTripleSetup::setup()
// remove #ifdef VERBOSE, #endif to print BASE-OT time

int main(int argc, const char** argv)
{
    std::vector<int> set_sizes;
    set_sizes.push_back((1 << 10));
    set_sizes.push_back((1 << 14));

    std::cout << "-------------------------------------------------" << std::endl;
    std::cout << "SEMIHONEST INVERSE OFFLINE TEST START" << std::endl;
    std::cout << "-------------------------------------------------" << std::endl;
    run<SemiShare>(argc, argv, MPC_BENCH::INVERSE_OFFLINE);

    std::cout << std::endl;
    std::cout << "-------------------------------------------------" << std::endl;
    std::cout << "SEMIHONEST GEN OFFLINE TEST START" << std::endl;
    std::cout << "-------------------------------------------------" << std::endl;
    run<SemiShare>(argc, argv, MPC_BENCH::GEN_OFFLINE);

    for (auto& set_size : set_sizes) {

        std::cout << std::endl;
        std::cout << "-------------------------------------------------" << std::endl;
        std::cout << "SEMIHONEST ONLINE TEST START (" << set_size << ")" << std::endl;
        std::cout << "-------------------------------------------------" << std::endl;
        run<SemiShare>(argc, argv, MPC_BENCH::ONLINE, set_size);

        std::cout << std::endl;
        std::cout << "-------------------------------------------------" << std::endl;
        std::cout << "SEMIHONEST EVAL OFFLINE TEST START (" << set_size << ")" << std::endl;
        std::cout << "-------------------------------------------------" << std::endl;
        run<SemiShare>(argc, argv, MPC_BENCH::EVAL_OFFLINE, set_size);
    }
}
