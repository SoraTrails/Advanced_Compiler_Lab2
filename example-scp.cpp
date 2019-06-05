#include "example-scp.h"

using namespace std;
int entrypc; // entry of program

vector<symbol> symbol_table;
map<symbol, vector<int> > use; 
map<symbol, vector<int> > def; 

vector<instruction> insts;
vector<function> funcs;

//use these two input to generate edges of cfg
map<int, int> entry;// entry of BB
set<pair<int,int> >edges;//original edge of instructions
/////////

/* 
 * read 3addr file, get all entry of bb
*/
void parse(FILE *fp){
    int bb_num=0; // num of bb
    int inst_id;
    char inst[32];
    char op1[32];
    char op2[32];
    int entry_func=0;
    int func_id=-1;
    set<pair<int,int> > br_target_1;

    while(fscanf(fp, "    instr %d: %s", &inst_id, inst) != EOF){
        instruction i;
        i.id = inst_id;
        strcpy(i.inst, inst);

        string ins(inst);
        switch (op_num[ins])
        {
        case 0:
            if(ins == "entrypc"){
                entrypc = inst_id;
            }
            i.op_num=0;
            i.op1[0] = '\0';
            i.op2[0] = '\0';
            break;
        case 1:
            fscanf(fp,"%s",op1);
            if(strcmp(inst, "enter") == 0){
                entry_func = inst_id;
                func_id++;
                auto res = entry.insert(pair<int,int>(inst_id,func_id));
                if(res.second)
                    bb_num++;
            }
            else if(strcmp(inst, "br") == 0){
                int tmp;
                sscanf(op1, "[%d]", &tmp);
                auto res = entry.insert(pair<int,int>(tmp,func_id));
                edges.insert(make_pair(inst_id, tmp));
                // if(strcmp(insts[tmp-2].inst, "call") != 0 && strcmp(insts[tmp-2].inst, "br") != 0)
                //     edges.insert(make_pair(tmp-1, tmp));
                br_target_1.insert(make_pair(tmp-1,tmp));
                if(res.second)
                    bb_num++;
            }
            else if(strcmp(inst, "call") == 0){
                auto res = entry.insert(pair<int,int>(inst_id+1,func_id));
                edges.insert(make_pair(inst_id, inst_id+1));
                if(res.second)
                    bb_num++;
            }
            else if(strcmp(inst, "ret") == 0){
                function f(bb_num, entry_func, inst_id);
                funcs.push_back(f);
                bb_num = 0; 
            }
            i.op_num=1;
            strcpy(i.op1, op1);
            i.op2[0] = '\0';
            break;
        case 2:
            fscanf(fp,"%s",op1);
            fscanf(fp,"%s",op2);
            if(strcmp(inst, "blbc") == 0 || strcmp(inst, "blbs") == 0){
                auto res = entry.insert(pair<int,int>(inst_id+1,func_id));
                edges.insert(make_pair(inst_id, inst_id+1));
                if(res.second)
                    bb_num++;
                int tmp;
                sscanf(op2, "[%d]", &tmp);
                res = entry.insert(pair<int,int>(tmp,func_id));
                edges.insert(make_pair(inst_id, tmp));
                // if(strcmp(insts[tmp-2].inst, "call") != 0 && strcmp(insts[tmp-2].inst, "br") != 0)
                //     edges.insert(make_pair(tmp-1, tmp));
                br_target_1.insert(make_pair(tmp-1,tmp));
                if(res.second)
                    bb_num++;
            }
            i.op_num=2;
            strcpy(i.op1, op1);
            strcpy(i.op2, op2);
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

    for(auto iter = br_target_1.begin();iter!=br_target_1.end();iter++){
        if(strcmp(insts[iter->first-1].inst, "call") != 0 && strcmp(insts[iter->first-1].inst, "br") != 0){
            edges.insert(make_pair(iter->first, iter->second));
        }
    }
}

/* 
 * construct cfg, entry,edge->function.list
 */
void construct_cfg(){
    int i=0;
    int func_id=0;
    for(auto iter = entry.begin(); iter != entry.end(); iter++) {
        if(func_id != iter->second){
            func_id = iter->second;
            i=0;
        }

        funcs[iter->second].bb_list.insert(make_pair(iter->first, i++));
    }
    
    for(auto edge = edges.begin(); edge != edges.end(); edge++) {
        auto res = entry.find(edge->second);
        if(edge->first > res->first){
            while(res->first <= edge->first){
                res++;
            }
            if(res != entry.begin())
                res--;
        }
        else{
            while(res->first > edge->first){
                res--;
            }
        }
        int id1 = funcs[res->second].bb_list[res->first];
        int id2 = funcs[res->second].bb_list[edge->second];
        funcs[res->second].list[id1].push_back(edge->second);
        funcs[res->second].rev_list[id2].push_back(res->first);
    }
}

/* 
 * print cfg
 */
void print_cfg(){
    for(auto func = funcs.begin(); func != funcs.end(); func++) {
        printf("Function: %d\n", func->entry);
        printf("Basic blocks:");
        for(auto bb = func->bb_list.begin(); bb != func->bb_list.end(); bb++){
            printf(" %d", bb->first);
        }
        printf("\nCFG:\n");
        auto bb = func->bb_list.begin();
        for(int i=0;i<func->bb_num;i++){
            printf("%d ->", bb->first);
            for(int j = 0; j < func->list[i].size(); j++){
                printf(" %d",func->list[i][j]);
            }
            printf("\n");
            bb++;
        }
    }
}

int main(int argc, char*argv[]){
    FILE *fp = fopen(argv[1], "r");
    if(fp == NULL){
        printf("error open input file\n");
    }
    // char file_name[256];
    // fscanf(fp,"compiling %s",file_name);
    parse(fp);
    construct_cfg();
    print_cfg();
    // for(auto iter = entry.begin(); iter != entry.end(); iter++) {
    //     printf("%d\n", iter->first);
    // }
}