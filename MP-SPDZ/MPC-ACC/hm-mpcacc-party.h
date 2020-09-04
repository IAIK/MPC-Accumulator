/*
 * ot.mpcacc-party.h
 *
 */

#include "Processor/OnlineOptions.h"
#include "Networking/Server.h"
#include "Networking/CryptoPlayer.h"
#include "Math/gfp.h"
#include "MPC-ACC/test.h"
#include "MPC-ACC/RCurve.h"

#include "MPC-ACC/MPCAcc.h"
#include "Tools/Bundle.h"

#include "Protocols/MaliciousRepMC.hpp"
#include "Protocols/MaliciousRepPrep.hpp"
#include "Protocols/Beaver.hpp"
#include "Protocols/fake-stuff.hpp"
#include "Processor/Input.hpp"
#include "Processor/Processor.hpp"
#include "Processor/Data_Files.hpp"
#include "GC/ShareSecret.hpp"
#include "GC/RepPrep.hpp"
#include "GC/ThreadMaster.hpp"

#include <assert.h>

#include "bench_acc.h"


template<template<class U> class T>
void run(int argc, const char** argv, MPC_BENCH b = MPC_BENCH::VERIFY, int set = 1000) {
    bigint::init_thread();
    ez::ezOptionParser opt;
    osuCrypto::REllipticCurve curve(B12_P381, RLC_EP_MTYPE);
    curve.printParam();
    curve.init_field();

    //----------------------------------------------------------------------
    if (b == MPC_BENCH::VERIFY) {
        std::cout << std::endl;
        if (testNonMPCAcc(curve))
            std::cout << "Test passed" << std::endl;
        else
            std::cout << "Test failed..." << std::endl;

        std::cout << std::endl;
        if (testNonMPCAccKeyless(curve))
            std::cout << "Test passed" << std::endl;
        else
            std::cout << "Test failed..." << std::endl;
        std::cout << std::endl;
    }
    //----------------------------------------------------------------------

    std::cout << "MPC setup..." << std::endl;
#ifdef DEBUG_MPCACC
    Timer timer;
    timer.start();
#endif
    Names N(opt, argc, argv, 3);
    CryptoPlayer P(N);
    int n_triples = set;
    // if (not opt.lastArgs.empty())
    //     n_triples = atoi(opt.lastArgs[0]->c_str());

    typedef typename MPC_ACC::Acc<T>::sShare share;

    if (b == MPC_BENCH::VERIFY) {
        n_triples = 20;
    }
    else if (b == MPC_BENCH::ONLINE) {
        n_triples = 20;
    }
    else if (b == MPC_BENCH::EVAL_OFFLINE) {
        n_triples = set + 1;
    }
    else if (b == MPC_BENCH::INVERSE_OFFLINE) {
        n_triples = 2;
    }
    else if (b == MPC_BENCH::GEN_OFFLINE) {
        n_triples = 1;
    }
    else {
        std::cout << "WRONG BENCH OPTION. ABORT;" << std::endl;
        return;
    }
    OnlineOptions::singleton.batch_size = n_triples;

    DataPositions usage;
    typename share::MAC_Check MCp;
    ArithmeticProcessor _({}, 0);
    auto& prep = *Preprocessing<share>::get_live_prep(0, usage);
    SubProcessor<share> proc(_, MCp, prep, P);

    MPC_ACC::Acc<T> acc_class(curve, proc, n_triples);
    std::cout << "Finished MPC setup" << std::endl;
#ifdef DEBUG_MPCACC
    std::cout << "Setup took " << timer.elapsed() * 1e3 << " ms for " << std::endl;
#endif

    if (b == MPC_BENCH::VERIFY) {
        acc_class.preprocessing(6);
        if (online_test(acc_class))
            std::cout << "Test passed" << std::endl;
        else
            std::cout << "Test failed..." << std::endl;
        return;
    }

    if (b == MPC_BENCH::ONLINE) {
        acc_class.preprocessing(6);
        if (online_benchmark(acc_class, P, set))
            std::cout << "Test passed" << std::endl;
        else
            std::cout << "Test failed..." << std::endl;
        return;
    }

     if (b == MPC_BENCH::EVAL_OFFLINE) {
        if (offline_benchmark(acc_class, P, 1))
            std::cout << "Test passed" << std::endl;
        else
            std::cout << "Test failed..." << std::endl;
        return;
    }

    if (b == MPC_BENCH::INVERSE_OFFLINE) {
        if (offline_benchmark(acc_class, P, 1))
            std::cout << "Test passed" << std::endl;
        else
            std::cout << "Test failed..." << std::endl;
        return;
    }

    if (b == MPC_BENCH::GEN_OFFLINE) {
        if (offline_benchmark(acc_class, P, 1))
            std::cout << "Test passed" << std::endl;
        else
            std::cout << "Test failed..." << std::endl;
        return;
    }

    std::cout << "WRONG BENCH OPTION. ABORT;" << std::endl;
}
