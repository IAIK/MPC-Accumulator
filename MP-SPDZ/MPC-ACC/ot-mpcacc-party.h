/*
 * ot.mpcacc-party.h
 *
 */

#include "Networking/Server.h"
#include "Networking/CryptoPlayer.h"
#include "Math/gfp.h"
#include "MPC-ACC/test.h"
#include "MPC-ACC/RCurve.h"

#include "MPC-ACC/MPCAcc.h"
#include "Protocols/SemiShare.h"
#include "Processor/BaseMachine.h"

#include "Protocols/Beaver.hpp"
#include "Protocols/fake-stuff.hpp"
#include "Protocols/MascotPrep.hpp"
#include "Processor/Processor.hpp"
#include "Processor/Data_Files.hpp"
#include "Processor/Input.hpp"

#include <assert.h>

#include "bench_acc.h"

template<template<class U> class T>
void run(int argc, const char** argv, MPC_BENCH b = MPC_BENCH::VERIFY, int set = 1000)
{
    ez::ezOptionParser opt;
    opt.add(
            "", // Default.
            0, // Required?
            0, // Number of args expected.
            0, // Delimiter if expecting multiple args.
            "Use SimpleOT instead of OT extension", // Help description.
            "-S", // Flag token.
            "--simple-ot" // Flag token.
    );
    opt.add(
            "", // Default.
            0, // Required?
            0, // Number of args expected.
            0, // Delimiter if expecting multiple args.
            "Don't check correlation in OT extension (only relevant with MASCOT)", // Help description.
            "-U", // Flag token.
            "--unchecked-correlation" // Flag token.
    );
    opt.add(
            "", // Default.
            0, // Required?
            0, // Number of args expected.
            0, // Delimiter if expecting multiple args.
            "Fewer rounds for authentication (only relevant with MASCOT)", // Help description.
            "-A", // Flag token.
            "--auth-fewer-rounds" // Flag token.
    );
    opt.add(
            "", // Default.
            0, // Required?
            0, // Number of args expected.
            0, // Delimiter if expecting multiple args.
            "Use Fiat-Shamir for amplification (only relevant with MASCOT)", // Help description.
            "-H", // Flag token.
            "--fiat-shamir" // Flag token.
    );
    opt.add(
            "", // Default.
            0, // Required?
            0, // Number of args expected.
            0, // Delimiter if expecting multiple args.
            "Skip sacrifice (only relevant with MASCOT)", // Help description.
            "-E", // Flag token.
            "--embrace-life" // Flag token.
    );
    opt.add(
            "", // Default.
            0, // Required?
            0, // Number of args expected.
            0, // Delimiter if expecting multiple args.
            "No MACs (only relevant with MASCOT; implies skipping MAC checks)", // Help description.
            "-M", // Flag token.
            "--no-macs" // Flag token.
    );

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
    Names N(opt, argc, argv, 2);
    PlainPlayer P(N);
    int n_triples = set;
    // if (not opt.lastArgs.empty())
    //     n_triples = atoi(opt.lastArgs[0]->c_str());
    BaseMachine machine;
    machine.ot_setups.push_back({P, true});

    typedef typename MPC_ACC::Acc<T>::sShare share;

    REllipticCurve::Scalar keyp;
    SeededPRNG G;
    keyp.randomize(G);

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
    typename share::Direct_MC MCp(keyp, N, 0);
    ArithmeticProcessor _({}, 0);
    typename share::LivePrep prep(0, usage);
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
