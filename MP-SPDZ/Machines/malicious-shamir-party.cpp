/*
 * malicious-shamir-party.cpp
 *
 */

#include "Machines/ShamirMachine.h"
#include "Protocols/MaliciousShamirShare.h"

int main(int argc, const char** argv)
{
    ShamirMachineSpec<MaliciousShamirShare>(argc, argv);
}
