#include "example-scp.h"

using namespace std;
int entrypc; // entries of program

vector<symbol> symbol_table;
vector<instruction> insts;
vector<Function> funcs;

//use these three input to generate edges of cfg
set<int> entries;// entries of BB
set<pair<int,int> >edges;//original edge of instructions
set<pair<int,int> >potential_edges;
/////////

/* 
 * read 3addr file, get all entries of bb
*/
void parse(FILE *fp){
    set<symbol> tmp_symbol_table;
    int inst_id;
    char inst[32];
    char op1[32];
    char op2[32];
    int entry_func;

    while(fscanf(fp, "    instr %d: %s", &inst_id, inst) != EOF){
        instruction i(inst);
        i.id = inst_id;
        switch (op_num[i.op_code])
        {
        case 0:
            if(strcmp(inst, "entrypc") == 0){
                entrypc = inst_id;
            }
            i.op_num=0;
            i.op1 = "";
            i.op2 = "";
            break;
        case 1:
            fscanf(fp,"%s",op1);
            if(strcmp(inst, "enter") == 0){
                entry_func = inst_id;
                entries.insert(inst_id);
            }
            else if(strcmp(inst, "br") == 0){
                int tmp;
                sscanf(op1, "[%d]", &tmp);
                entries.insert(tmp);
                edges.insert(make_pair(inst_id, tmp));
                potential_edges.insert(make_pair(tmp-1,tmp));
            }
            else if(strcmp(inst, "call") == 0){
                entries.insert(inst_id+1);
                edges.insert(make_pair(inst_id, inst_id+1));
            }
            else if(strcmp(inst, "ret") == 0){
                Function f(entry_func, inst_id);
                funcs.push_back(f);
            }
            i.op_num=1;
            i.op1 = string(op1);
            i.op2 = "";
            break;
        case 2:
            fscanf(fp,"%s",op1);
            fscanf(fp,"%s",op2);
            if(strcmp(inst, "blbc") == 0 || strcmp(inst, "blbs") == 0){
                entries.insert(inst_id+1);
                edges.insert(make_pair(inst_id, inst_id+1));
                int tmp;
                sscanf(op2, "[%d]", &tmp);
                entries.insert(tmp);
                edges.insert(make_pair(inst_id, tmp));
                potential_edges.insert(make_pair(tmp-1,tmp));
            }
            i.op_num=2;
            i.op1 = string(op1);
            i.op2 = string(op2);
            break;
        default:
            break;
        }
        insts.push_back(i);
    }
    // for(int i=0;i<insts.size();i++){
    //     printf("%s ", insts[i].inst);
    //     if(insts[i].op_num > 0){
    //         printf("%s ", insts[i].op1);
    //     }
    //     if(insts[i].op_num > 1){
    //         printf("%s", insts[i].op2);
    //     }
    //     printf("\n");
    // }

    for(auto iter = potential_edges.begin();iter!=potential_edges.end();iter++){
        if(insts[iter->first].op_code != "call" && insts[iter->first].op_code != "br"){
            edges.insert(make_pair(iter->first, iter->second));
        }
    }

    copy(tmp_symbol_table.begin(), tmp_symbol_table.end(), back_inserter(symbol_table));
    // basic_block::sym_num = symbol_table.size();
}

/* 
 * construct cfg, entries,edge->Function.list
 */
void construct_cfg(){
    // int i=0;
    // int func_id=0;

    //first cycle: store all bbs in functions
    auto f = funcs.begin();
    int i=0;
    for(auto entry = entries.begin(); entry != entries.end(); entry++) {
        if(*entry > f->end_ins){
            //should not reach funcs.end()
            assert(i == f->bbs.size());
            f++;
            i=0;
        }
        auto tmp = next(entry);
        if(tmp != entries.end()){
            f->bbs.push_back(basic_block(*entry, (*tmp)-1));
        }
        else{
            f->bbs.push_back(basic_block(*entry, f->end_ins));
        }
        f->bb_map.insert(make_pair(*entry,i++));
    }

    //second cycle: add pre & suc to bbs
    auto func = funcs.begin();
    for(auto edge = edges.begin(); edge != edges.end(); edge++) {
        while(edge->first > func->end_ins || edge->second > func->end_ins){
            func++;
        }
        auto res = entries.find(edge->second);
        //edge->first may not be a entry of a bb
        //edge->second must be a entry of a bb
        //following if-else finds the bb that the edge->first belongs to 
        if(edge->first > *res){
            while(*res <= edge->first){
                res++;
            }
            if(res != entries.begin())
                res--;
        }
        else{
            while(*res > edge->first){
                res--;
            }
        }
        int from = func->bb_map.at(*res);
        int to = func->bb_map.at(edge->second);
        func->bbs[from].suc.push_back(to);
        func->bbs[to].pre.push_back(from);
    }
}

/* 
 * print cfg
 */
void print_cfg(){
    for(auto func = funcs.begin(); func != funcs.end(); func++) {
        printf("Function: %d\n", func->start_ins);
        printf("Basic blocks:");
        for(auto bb = func->bbs.begin(); bb != func->bbs.end(); bb++){
            printf(" %d", bb->start_ins);
        }
        printf("\nCFG:\n");
        for(auto bb = func->bbs.begin(); bb != func->bbs.end(); bb++){
            printf("%d ->", bb->start_ins);
            for(auto b = bb->suc.begin(); b != bb->suc.end(); b++){ 
                printf(" %d", func->bbs[*b].start_ins);
            }
            printf("\n");
        }
    }
}

int main(int argc, char*argv[]){
    FILE *fp = fopen(argv[1], "r");
    if(fp == NULL){
        printf("error open input file\n");
    }
    insts.push_back(instruction());
    // char file_name[256];
    // fscanf(fp,"compiling %s",file_name);
    parse(fp);
    construct_cfg();
    print_cfg();
    // for(auto iter = entries.begin(); iter != entries.end(); iter++) {
    //     printf("%d\n", iter->first);
    // }
}