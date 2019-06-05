#include <stdio.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <map>
#include <set>
using namespace std;
/*
The 3-address format uses a simple RISC instruction set, and assumes an infinite number of virtual registers (we discuss the stack frame layout below in the Function ABI section). The operands to these instructions may be one of the following:

    GP (global pointer): A pointer to the beginning of the global address space.
    FP (frame pointer): A pointer to the beginning of the frame of the current function.
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

struct symbol
{
    char name[32];
};
std::map<std::string, int> op_num = {
    {"wrl",0}, {"entrypc",0}, {"nop",0},
    {"neg",1}, {"br",1}, {"call",1}, {"load",1},  {"enter",1},   {"ret",1}, {"param",1}, {"read",1}, {"write",1},
    {"store",2}, {"add",2}, {"sub",2}, {"mul",2}, {"div",2}, {"mod",2}, {"cmpeq",2},  {"cmple",2}, {"cmplt",2}, {"blbc",2}, {"blbs",2},{"move",2}
};
struct instruction
{
    int id;
    char inst[32];
    int op_num;
    char op1[32];
    char op2[32];
};

class function
{
public:
// private:
    int entry;
    int exit;
    int bb_num;
// public:
    vector<int> *list = NULL;
    vector<int> *rev_list = NULL;
    map<int,int> bb_list; // map from instid to bb_id

    function(){}
    function(int bb_num, int entry, int exit){
        this->bb_num = bb_num;
        this->entry = entry;
        this->exit = exit;
        list = new vector<int>[bb_num];
        rev_list = new vector<int>[bb_num];
    }
    function(const function & f){
        this->bb_num = f.bb_num;
        this->entry = f.entry;
        this->exit = f.exit;
        list = new vector<int>[this->bb_num];
        rev_list = new vector<int>[this->bb_num];
        for(int i=0;i<this->bb_num;i++){
            this->list[i] = f.list[i];
            this->rev_list[i] = f.rev_list[i];
        }
    }

    ~function(){
        if(list)
            delete []list;
        if(rev_list)
            delete []rev_list;
    }
};
