#include <stdio.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <map>
#include <set>
#include <bitset>
#include <algorithm>
#include <boost/dynamic_bitset.hpp>
using namespace std; 
/*
The 3-address format uses a simple RISC instruction set, and assumes an infinite number of virtual registers (we discuss the stack frame layout below in the Function ABI section). The operands to these instructions may be one of the following:

    GP (global pointer): A pointer to the beginning of the global address space.
    FP (frame pointer): A pointer to the beginning of the frame of the current Function.
    Constants: For example, 24
    Address offsets: In the name, global_array_base#32576, the number following the # is the starting address offset of variable global_array relative to the GP or FP. The variable names before the # here and below are to help humans read the representation.
    Field offsets: In y_offset#8, the number following the # is the starting address offset of field y relative to the starting address of the corresponding struct (p_base#32560 in this example).
    Local variables (scalars): For example, a#24 represents a local variable and its offset within the stack frame. You should ignore the offset for now (you will use it later for register allocation). You may assume that the variable a is allocated to the virtual register a.
    Register names: A (13) means virtual register r13. Arithmetic and some other instructions write their results to a register. (See below for the complete list.) For example, instruction 13 may write its result to r13.
    Instruction labels: The [46] label represents a instruction to jump to (in this example, jump to instruction 46).

Arithmetic instructions : add, sub, mul, div, mod, neg, cmpeq, cmple, cmplt 
Branch instructions : br, blbc, blbs, call
Data movement instructions : load, store, move(rs, rd)
I/O instructions : read, write, wrl
Other instructions : param, enter, ret, entrypc, nop

zero op: wrl, entrypc, nop
one op: neg, br, call, load, store, enter, ret, param, read, write
two op: add, sub, mul, div, mod, cmpeq, cmple, cmplt, blbc, blbs, move
*/
map<string, int> op_num = {
    {"wrl",0}, {"entrypc",0}, {"nop",0},
    {"neg",1}, {"br",1}, {"call",1}, {"load",1},  {"enter",1},   
    {"ret",1}, {"param",1}, {"read",1}, {"write",1},
    {"store",2}, {"add",2}, {"sub",2}, {"mul",2}, {"div",2}, 
    {"mod",2}, {"cmpeq",2},  {"cmple",2}, {"cmplt",2}, 
    {"blbc",2}, {"blbs",2},{"move",2}
};

//NO_USE_DEF
#define NUD 0
#define USE 1
#define DEF 2
map<string, int>ud1_table={
    {"neg",USE}, {"br",NUD}, {"call",NUD}, {"load",USE},  {"enter",NUD},   {"ret",NUD}, {"param",USE},{"read",DEF}, {"write",USE}
};
map<string, pair<int,int> >ud2_table={
    {"store",make_pair(USE,DEF)},   {"add",make_pair(USE,USE)}, 
    {"sub",make_pair(USE,USE)},     {"mul",make_pair(USE,USE)}, 
    {"div",make_pair(USE,USE)},     {"mod",make_pair(USE,USE)}, 
    {"cmpeq",make_pair(USE,USE)},   {"cmple",make_pair(USE,USE)}, 
    {"cmplt",make_pair(USE,USE)},   {"blbc",make_pair(USE,USE)}, 
    {"blbs",make_pair(USE,USE)},    {"move",make_pair(USE,DEF)}
};

#define LOCAL 0
#define GLOBAL 1
struct symbol
{
    string name;
    int type;
    int address;
    bool operator<(const symbol a) const{
        return 
        (this->name < a.name) || 
        (this->name == a.name && this->type < a.type);
    }
    bool operator==(const symbol a) const{
        return (this->name == a.name && this->type == a.type);
    }
    symbol(){}
    symbol(string &n,int addr,int t){
        if(t == GLOBAL){
            size_t index = n.rfind("_base");
            // if(index != std::string::npos){
                n.replace(index, 5, "");
            // }
        }
        type = t;
        name = n;
        address = addr;
    }
};

struct instruction
{
    int id;
    string op_code;
    int op_num;
    string op1;
    string op2;
    int use1=-1;//symbol_table index
    int use2=-1;
    int def=-1;
    instruction(){
        id = 0;
        op_code="nop";
        op_num = 0;
    }
    instruction(char *op_code){
        this->op_code = string(op_code);
    }
};

//instruction that performs a def,
//move and store are considered
//read is not considered
struct def_instruction
{
    int start_ins;
    int end_ins;
    int def;//symbol_table index
    vector<int> use;//symbol_table index
    long long value;
    def_instruction(){}
};

class basic_block
{
public:
    int start_ins;
    int end_ins;
    boost::dynamic_bitset<> use;
    boost::dynamic_bitset<> def;
    boost::dynamic_bitset<> gen;
    boost::dynamic_bitset<> kill;
    boost::dynamic_bitset<> in;
    boost::dynamic_bitset<> out;
    vector<int> pre;//存储在function中bbs的下标
    vector<int> suc;//存储在function中bbs的下标
    basic_block(int start_ins, int end_ins){
        this->start_ins = start_ins;
        this->end_ins = end_ins;
    }
};

class Function
{
public:
// private:
    int start_ins;
    int end_ins;
    vector<basic_block> bbs;
    map<int, int> bb_map;
    vector<symbol> symbol_table;
    vector<def_instruction> def_ins_table;
// public:
    Function(){}
    Function(int start_ins, int end_ins){
        this->start_ins = start_ins;
        this->end_ins = end_ins;
    }
    ~Function(){}
};
