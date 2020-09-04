#pragma once

#include "MPCAcc.h"

enum MPC_BENCH {
    VERIFY, ONLINE, EVAL_OFFLINE, INVERSE_OFFLINE, GEN_OFFLINE
};

#define ONLINE_RUNS 50
#define OFFLINE_RUNS 1

// make sure enough triples are produced!
// In ReplicatedPrep.hpp:
//    void BufferPrep<T>::get_three_no_count(Dtype dtype, T& a, T& b, T& c)
//    void BufferPrep<T>::get_two_no_count(Dtype dtype, T& a, T& b)
// remove the pop_back function and call mpc_acc.preprocessing(3)
template<template<class U> class T>
bool online_benchmark(MPC_ACC::Acc<T>& acc_class, Player& P, int set_size)
{
    using MPC_ACC::Acc;
    using MPC_ACC::Auxillary;
    using MPC_ACC::Witness;

    Timer timer;

    double gen_time = 0;
    double eval_time = 0;
    double wit_create_time = 0;
    double add_time = 0;
    double wit_update_add_time = 0;
    double delete_time = 0;
    double wit_update_delete_time = 0;

    NamedCommStats stats;

    typename Acc<T>::Accumulator acc;
    Auxillary<T> aux;
    typename  Acc<T>::Set el;
    acc_class.genSet(el, set_size);
    Witness<T> wit;
    typename Acc<T>::Element new_el;
    acc_class.genElement(new_el, set_size + 1);
    Witness<T> old_wit;
    typename Acc<T>::Wit tmp1;
    typename Acc<T>::Element tmp2;

    for (int runs = 0; runs < ONLINE_RUNS; runs++)
    {
        std::cout << "-------------------------------------------------" << std::endl;
        std::cout << "Run: " << (runs + 1) << " of " << ONLINE_RUNS << std::endl;
        std::cout << "-------------------------------------------------" << std::endl;

        double tmp_time;

        stats = P.comm_stats;
        timer.start();
        acc_class.Gen();
        timer.stop();
        tmp_time = timer.elapsed() * 1e3;
        gen_time += tmp_time;
        std::cout << "Gen time: " << tmp_time << " ms" << std::endl;
        timer.reset();
        (P.comm_stats - stats).print(true);
        std::cout << std::endl;

        stats = P.comm_stats;
        timer.start();
        acc_class.Eval(el, acc, aux);
        timer.stop();
        tmp_time = timer.elapsed() * 1e3;
        eval_time += tmp_time;
        std::cout << "Eval time: " << tmp_time << " ms" << std::endl;
        timer.reset();
        (P.comm_stats - stats).print(true);
        std::cout << std::endl;

        stats = P.comm_stats;
        timer.start();
        bool ret = acc_class.WitCreate(acc, aux, el[0], wit);
        timer.stop();
        tmp_time = timer.elapsed() * 1e3;
        if(!ret)
            return false;
        wit_create_time += tmp_time;
        std::cout << "WitCreate time: " << tmp_time << " ms" << std::endl;
        timer.reset();
        (P.comm_stats - stats).print(true);
        std::cout << std::endl;

        if (!acc_class.Verify(acc, el[0], wit))
            return false;
        if (acc_class.Verify(acc, el[1], wit))
            return false;

        stats = P.comm_stats;
        timer.start();
        ret = acc_class.Add(acc, aux, new_el);
        timer.stop();
        tmp_time = timer.elapsed() * 1e3;
        if (!ret)
            return false;
        add_time += tmp_time;
        std::cout << "Add time: " << tmp_time << " ms" << std::endl;
        timer.reset();
        (P.comm_stats - stats).print(true);
        std::cout << std::endl;

        tmp1 = wit.getWitness();
        tmp2 = wit.getElement();
        old_wit.init(tmp1, tmp2);

        stats = P.comm_stats;
        timer.start();
        ret = acc_class.WitUpdate(wit, aux, new_el);
        timer.stop();
        tmp_time = timer.elapsed() * 1e3;
        if (!ret)
            return false;
        wit_update_add_time += tmp_time;
        std::cout << "WitUpdate Add time: " << tmp_time << " ms" << std::endl;
        timer.reset();
        (P.comm_stats - stats).print(true);
        std::cout << std::endl;

        if (!acc_class.Verify(acc, el[0], wit))
            return false;
        if (acc_class.Verify(acc, el[0], old_wit))
            return false;

        stats = P.comm_stats;
        timer.start();
        ret = acc_class.Delete(acc, aux, el[2]);
        timer.stop();
        tmp_time = timer.elapsed() * 1e3;
        if (!ret)
            return false;
        delete_time += tmp_time;
        std::cout << "Delete time: " << tmp_time << " ms" << std::endl;
        timer.reset();
        (P.comm_stats - stats).print(true);
        std::cout << std::endl;

        tmp1 = wit.getWitness();
        tmp2 = wit.getElement();
        old_wit.init(tmp1, tmp2);


        stats = P.comm_stats;
        timer.start();
        ret = acc_class.WitUpdate(wit, aux, el[2]);
        timer.stop();
        tmp_time = timer.elapsed() * 1e3;
        if (!ret)
            return false;
        wit_update_delete_time += tmp_time;
        std::cout << "WitUpdate Del time: " << tmp_time << " ms" << std::endl;
        timer.reset();
        (P.comm_stats - stats).print(true);
        std::cout << std::endl;

        if (!acc_class.Verify(acc, el[0], wit))
            return false;
        if (acc_class.Verify(acc, el[0], old_wit))
            return false;
    }

    std::cout << "-------------------------------------------------" << std::endl;
    std::cout << "Average for " << ONLINE_RUNS << " runs" << std::endl;
    std::cout << "-------------------------------------------------" << std::endl;
    std::cout << "Gen time: " << gen_time / ONLINE_RUNS << " ms" << std::endl;
    std::cout << "Eval time: " << eval_time / ONLINE_RUNS << " ms" << std::endl;
    std::cout << "WitCreate time: " << wit_create_time / ONLINE_RUNS << " ms" << std::endl;
    std::cout << "Add time: " << add_time / ONLINE_RUNS << " ms" << std::endl;
    std::cout << "WitUpdateAdd time: " << wit_update_add_time / ONLINE_RUNS << " ms" << std::endl;
    std::cout << "Delete time: " << delete_time / ONLINE_RUNS << " ms" << std::endl;
    std::cout << "WitUpdateDel time: " << wit_update_delete_time / ONLINE_RUNS << " ms" << std::endl;
    return true;
}

template<template<class U> class T>
bool offline_benchmark(MPC_ACC::Acc<T>& acc_class, Player& P, int inverses)
{
    Timer timer;

    std::cout << "Initial communication:" << std::endl;
    P.comm_stats.print(true);
    std::cout << std::endl;

    double time = 0;

    auto& prep = acc_class.getProc().DataF;
    size_t start;
    for (int runs = 0; runs < OFFLINE_RUNS; runs++)
    {
        std::cout << "-------------------------------------------------" << std::endl;
        std::cout << "Run: " << (runs + 1) << " of " << OFFLINE_RUNS << std::endl;
        std::cout << "-------------------------------------------------" << std::endl;

        double tmp_time;

        start = P.sent + prep.data_sent();
        timer.start();
        acc_class.preprocessing(inverses);
        timer.stop();
        tmp_time = timer.elapsed() * 1e3;
        time += tmp_time;
        std::cout << "Time: " << tmp_time << " ms" << std::endl;
        timer.reset();
        std::cout << 1e-3 * (P.sent + prep.data_sent() - start) << " kbytes sent" << std::endl;
    }

    std::cout << "-------------------------------------------------" << std::endl;
    std::cout << "Average for " << OFFLINE_RUNS << " runs" << std::endl;
    std::cout << "-------------------------------------------------" << std::endl;
    std::cout << "Time: " << time / OFFLINE_RUNS << " ms" << std::endl;
    return true;
}

template<template<class U> class T>
bool online_test(MPC_ACC::Acc<T>& acc_class)
{
    using MPC_ACC::Acc;
    using MPC_ACC::Auxillary;
    using MPC_ACC::Witness;

    std::cout << "Testing MPC Accumulator" << std::endl;
    acc_class.Gen();
    typename Acc<T>::Accumulator acc;
    Auxillary<T> aux;
    typename  Acc<T>::Set el;
    acc_class.genSet(el, SET_LENGTH);
    acc_class.Eval(el, acc, aux);
    Witness<T> wit;
    if(!acc_class.WitCreate(acc, aux, el[0], wit))
        return false;

    if (!acc_class.Verify(acc, el[0], wit))
        return false;
    if (acc_class.Verify(acc, el[1], wit))
        return false;

    typename Acc<T>::Element new_el;
    acc_class.genElement(new_el, el.size());

    if (!acc_class.Add(acc, aux, new_el))
      return false;

    Witness<T> old_wit;
    typename Acc<T>::Wit tmp1 = wit.getWitness();
    typename Acc<T>::Element tmp2 = wit.getElement();
    old_wit.init(tmp1, tmp2);
    if (!acc_class.WitUpdate(wit, aux, new_el))
        return false;

    if (!acc_class.Verify(acc, el[0], wit))
        return false;
    if (acc_class.Verify(acc, el[0], old_wit))
        return false;

    if (!acc_class.Delete(acc, aux, el[2]))
      return false;

    tmp1 = wit.getWitness();
    tmp2 = wit.getElement();
    old_wit.init(tmp1, tmp2);
    if (!acc_class.WitUpdate(wit, aux, el[2]))
        return false;

    if (!acc_class.Verify(acc, el[0], wit))
        return false;
    if (acc_class.Verify(acc, el[0], old_wit))
        return false;

    if (!acc_class.Delete(acc, aux, el[0]))
      return false;

    if (acc_class.WitUpdate(wit, aux, el[0]))
        return false;

    if (acc_class.Verify(acc, el[0], wit))
        return false;

    return true;
}
