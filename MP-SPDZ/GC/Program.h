/*
 * Program.h
 *
 */

#ifndef GC_PROGRAM_H_
#define GC_PROGRAM_H_

#include "GC/Instruction.h"
#include "Processor/Program.h"

#include <vector>
using namespace std;

namespace GC
{

enum BreakType {
    TIME_BREAK,
    DONE_BREAK,
    CAP_BREAK,
    CLEANING_BREAK,
};

template <class T> class Processor;

template <class T>
class Program
{
    vector<Instruction> p;
    int offline_data_used;

    // Maximal register used
    unsigned max_reg[MAX_REG_TYPE];

    // Memory size used directly
    unsigned max_mem[MAX_REG_TYPE];

    // True if program contains variable-sized loop
    bool unknown_usage;

    void compute_constants();

    public:

    Program();

    // Read in a program
    void parse_file(const string& filename);
    void parse(const string& programe);
    void parse(istream& s);

    int get_offline_data_used() const { return offline_data_used; }
    void print_offline_cost() const;

    bool usage_unknown() const { return unknown_usage; }

    unsigned num_reg(RegType reg_type) const
      { return max_reg[reg_type]; }

    const unsigned* direct_mem(RegType reg_type) const
      { return &max_mem[reg_type]; }

    template<class U>
    BreakType execute(Processor<T>& Proc, U& dynamic_memory, int PC = -1) const;

    bool done(Processor<T>& Proc) const { return Proc.PC >= p.size(); }

    template <class U>
    Program<U>& cast() { return *reinterpret_cast< Program<U>* >(this); }
};


} /* namespace GC */

#endif /* GC_PROGRAM_H_ */
