#include "dfa.h"

using namespace std;
int entrypc; // entries of program

// vector<symbol> symbol_table;
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
    // set<symbol> tmp_symbol_table;
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

    // copy(tmp_symbol_table.begin(), tmp_symbol_table.end(), back_inserter(symbol_table));
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

/* 
 * print symbol_table
 */
void print_symbol_table(){
    printf("symbol table:\n");
    for(auto f:funcs) {
        printf("Func %d\n",f.start_ins);
        for(auto i:f.symbol_table){
            printf("%s,%d\n", i.name.c_str(),i.type);
        }
        printf("\n");
    }
}

/* 
 * print def_ins_table
 */
void print_def_ins_table(){
    printf("def_ins table:\n");
    for(auto f:funcs) {
        printf("Func %d\n",f.start_ins);
        for(auto i:f.def_ins_table){
            printf("%d-%d:%s --- ", i.start_ins,i.end_ins, f.symbol_table[i.def].name.c_str());
            for(auto j:i.use){
                printf("%s ", f.symbol_table[j].name.c_str());
            }
            if(i.use.size() == 0){
                printf("%d", (int)i.value);
            }
            printf("\n");
        }
        printf("\n");
    }
}
/* 
 * print bit vector
 */
void print_bit_vector(int type){
    if(type == 0){
        printf("bit vector:\n");
        for(auto f:funcs) {
            printf("Func %d\n",f.start_ins);
            for(auto bb:f.bbs){
                string gen;
                string kill;
                boost::to_string(bb.gen,gen);
                boost::to_string(bb.kill,kill);

                printf("basic block %d-%d: gen:%s; kill:%s\n",bb.start_ins,bb.end_ins, gen.c_str(), kill.c_str());
            }
            printf("\n");
        }
    }
    else if(type == 1){
        printf("bit vector:\n");
        for(auto f:funcs) {
            printf("Func %d\n",f.start_ins);
            printf("symbol: ");
            for(auto s:f.symbol_table){
                printf("%s ",s.name.c_str());
            }
            printf("\n");
            for(auto bb:f.bbs){
                string use;
                string def;
                boost::to_string(bb.use,use);
                boost::to_string(bb.def,def);

                printf("basic block %d-%d: use:%s; def:%s\n",bb.start_ins,bb.end_ins, use.c_str(), def.c_str());
            }
            printf("\n");
        }
    }
}
/* 
 * print in out of basic blocks
 */
void print_in_out(){
    printf("IN OUT:\n");
    for(auto f:funcs) {
        printf("Func %d\n",f.start_ins);
        for(auto bb:f.bbs){
            string in;
            string out;
            boost::to_string(bb.in,in);
            boost::to_string(bb.out,out);

            printf("basic block %d-%d: in:%s; out:%s\n",bb.start_ins,bb.end_ins, in.c_str(), out.c_str());
        }
        printf("\n");
    }
}

/* 
 * print optimized 3addr
 */
void print_optimized_3addr(){
    printf("Optimized 3addr:\n");
    for(auto f:funcs) {
        printf("Func %d\n",f.start_ins);
        for(auto bb:f.bbs){
            for(int i=bb.start_ins;i<=bb.end_ins;i++){
                printf("insts[%2d]: %s ", i, insts[i].op_code.c_str());
                if(insts[i].op_num > 0){
                    printf("%s ", insts[i].op1.c_str());
                }
                if(insts[i].op_num > 1){
                    printf("%s", insts[i].op2.c_str());
                }
                printf("\n");
            }
        }
        printf("\n");
    }
}

/* 
 * construct symbol table
 */
void construct_symbol_table(){
    for(auto f = funcs.begin();f != funcs.end();f++){
        // set<symbol> st;

        for(auto bb = f->bbs.begin();bb != f->bbs.end();bb++){
            for(int i=bb->start_ins;i<=bb->end_ins;i++){
                size_t index;
                if (insts[i].op_num == 2)
                {
                    index = insts[i].op1.find('#');
                    if(index != string::npos){
                        string tmp = insts[i].op1.substr(0,index);
                        int addr = stoi(insts[i].op1.substr(index+1));
                        // sscanf(insts[i].op1.c_str(),"%s#%d",tmp,&addr);
                        if(insts[i].op2 == "GP"){
                            symbol t(tmp,addr,GLOBAL);
                            auto res = find(f->symbol_table.begin(),f->symbol_table.end(),t);
                            if (res == f->symbol_table.end()){
                                f->symbol_table.push_back(t);
                                insts[i].use1 = f->symbol_table.size()-1;
                            }
                            else{
                                insts[i].use1 = res - f->symbol_table.begin();
                            }
                        }
                        else{
                            symbol t(tmp,addr,LOCAL);
                            auto res = find(f->symbol_table.begin(),f->symbol_table.end(),t);
                            if (res == f->symbol_table.end()){
                                f->symbol_table.push_back(t);
                                insts[i].use1 = f->symbol_table.size()-1;
                            }
                            else{
                                insts[i].use1 = (res-f->symbol_table.begin());
                            }                        
                        }
                    }
                    index = insts[i].op2.find('#');
                    if(index != string::npos){
                        string tmp = insts[i].op2.substr(0,index);
                        int addr = stoi(insts[i].op2.substr(index+1));
                        // sscanf(insts[i].op2.c_str(),"%s#%d",tmp,&addr);
                        symbol t(tmp,addr,LOCAL);

                        // store op2 must be an address of global variable
                        // move op2 must be a local variable
                        if(/*insts[i].op_code == "store" || */insts[i].op_code == "move"){
                            auto res = find(f->symbol_table.begin(),f->symbol_table.end(),t);
                            if (res == f->symbol_table.end()){
                                f->symbol_table.push_back(t);
                                insts[i].def = f->symbol_table.size()-1;
                            }
                            else{
                                insts[i].def = (res-f->symbol_table.begin());
                            }
                        }
                        else{
                            auto res = find(f->symbol_table.begin(),f->symbol_table.end(),t);
                            if (res == f->symbol_table.end()){
                                f->symbol_table.push_back(t);
                                insts[i].use2 = f->symbol_table.size()-1;
                            }
                            else{
                                insts[i].use2 = (res-f->symbol_table.begin());
                            }
                        }
                    }
                }
                else if (insts[i].op_num == 1){
                    index = insts[i].op1.find('#');
                    if(index != string::npos){
                        string tmp = insts[i].op1.substr(0,index);
                        int addr = stoi(insts[i].op1.substr(index+1));
                        // sscanf(insts[i].op1.c_str(),"%s#%d",tmp,&addr);
                        symbol t(tmp,addr,LOCAL);
                        if(ud1_table.at(insts[i].op_code) == USE){
                            auto res = find(f->symbol_table.begin(),f->symbol_table.end(),t);
                            if (res == f->symbol_table.end()){
                                f->symbol_table.push_back(t);
                                insts[i].use1 = f->symbol_table.size()-1;
                            }
                            else{
                                insts[i].use1 = res - f->symbol_table.begin();
                            }
                        }
                    }
                }
            }
        }

        // copy(st.begin(), st.end(), back_inserter(f->symbol_table));
    }
}
/* 
 * go upstream to find the symbol of address
 */
void upstream(vector<int> &op, int index, int &min_ins){
    long long a;
    int r = sscanf(insts[index].op1.c_str(), "(%lld)", &a);
    if(r == 1){
        upstream(op,(int)a,min_ins);
    }
    else if(insts[index].use1 != -1){
        min_ins = min(min_ins, index);
        op.push_back(insts[index].use1);
    }

    r = sscanf(insts[index].op2.c_str(), "(%lld)", &a);
    if(r == 1){
        upstream(op,(int)a,min_ins);
    }
    else if(insts[index].use2 != -1){
        min_ins = min(min_ins, index);
        op.push_back(insts[index].use2);
    }
}

/* 
 * go upstream to find the def symbol of address
 */
void upstream_def(int &op, int index, int &min_ins){
    long long a;
    int r = sscanf(insts[index].op1.c_str(), "(%lld)", &a);
    if(r == 1){
        upstream_def(op,(int)a,min_ins);
    }
    else if(insts[index].use1 != -1){
        min_ins = min(min_ins, index);
        op = insts[index].use1;
    }

    r = sscanf(insts[index].op2.c_str(), "(%lld)", &a);
    if(r == 1){
        upstream_def(op,(int)a,min_ins);
    }
    else if(insts[index].use2 != -1){
        min_ins = min(min_ins, index);
        op = insts[index].use2;
    }
}

/* 
 * go upstream to find the use symbol of address
 */
void upstream_use(set<int> &op, int index, set<int> &def){
    long long a;
    int r = sscanf(insts[index].op1.c_str(), "(%lld)", &a);
    if(r == 1){
        upstream_use(op,(int)a, def);
    }
    else if(insts[index].use1 != -1){
        if(def.find(insts[index].use1) == def.end())
            op.insert(insts[index].use1);
    }
    r = sscanf(insts[index].op2.c_str(), "(%lld)", &a);
    if(r == 1){
        upstream_use(op,(int)a, def);
    }
    else if(insts[index].use2 != -1){
        if(def.find(insts[index].use2) == def.end())
            op.insert(insts[index].use2);
    }
}

/* 
 * construct def_ins table
 */
void construct_def_ins_table(){
    for(auto f = funcs.begin();f != funcs.end();f++){
        for(auto bb = f->bbs.begin();bb != f->bbs.end();bb++){
            for(int i=bb->start_ins;i<=bb->end_ins;i++){
                def_instruction tmp;
                // move op2 must be a local variable
                if(insts[i].op_code == "move"){
                    tmp.end_ins = i;
                    tmp.def = insts[i].def;
                    //op1 is a symbol
                    if(insts[i].use1 != -1){
                        tmp.start_ins = i;
                        tmp.use.push_back(insts[i].use1);
                    }
                    //op1 is a const
                    else {
                        long long a;
                        int r = sscanf(insts[i].op1.c_str(), "%lld", &a);
                        if(r == 1){
                            tmp.start_ins = i;
                            tmp.value = a;
                        }
                        //op1 is a address, need to go upstream
                        else{
                            sscanf(insts[i].op1.c_str(),"(%lld)",&a);
                            vector<int>op;
                            int min_ins=i;
                            upstream(op,(int)a,min_ins);
                            tmp.start_ins = min_ins;
                            tmp.use.insert(tmp.use.end(), op.begin(), op.end());
                        }
                    }
                }
                // store op2 must be an address of global variable
                // op2 must go upstream
                // op1 may also go upstream
                else if(insts[i].op_code == "store"){
                    long long t;
                    tmp.end_ins = i;
                    sscanf(insts[i].op2.c_str(),"(%lld)",&t);
                    int op1;
                    int min_ins=i;
                    upstream_def(op1,(int)t,min_ins);
                    tmp.def = op1;
                    //op1 is a symbol
                    if(insts[i].use1 != -1){
                        tmp.start_ins = i;
                        tmp.use.push_back(insts[i].use1);
                    }
                    //op1 is a const
                    else {
                        long long a;
                        int r = sscanf(insts[i].op1.c_str(), "%lld", &a);
                        if(r == 1){
                            tmp.start_ins = min(i,min_ins);
                            tmp.value = a;
                        }
                        //op1 is a address, need to go upstream
                        else{
                            sscanf(insts[i].op1.c_str(),"(%lld)",&a);
                            vector<int>op;
                            int min_ins_tmp=i;
                            upstream(op,(int)a,min_ins_tmp);
                            tmp.start_ins = min(min_ins_tmp,min_ins);
                            tmp.use.insert(tmp.use.end(), op.begin(), op.end());
                        }
                    }
                }
                else{
                    continue;
                }
                f->def_ins_table.push_back(tmp);
            }
        }
    }
}

/* 
 * generate gen kill bit vector of basic block
 */
void generate_gen_kill_bit_vec_of_bb(){
    for(auto f = funcs.begin();f != funcs.end();f++){
        auto def_ins = f->def_ins_table.begin();
        for(auto bb = f->bbs.begin();bb != f->bbs.end();bb++){
            int start_ins = bb->start_ins;
            int end_ins = bb->end_ins;
            //存放def_instruction在def_ins_table中的下标
            set<int>gen;
            set<int>kill;
            bb->gen.resize(f->def_ins_table.size(),false);
            bb->kill.resize(f->def_ins_table.size(),false);
            
            if(def_ins == f->def_ins_table.end()){
                continue;
            }
            while(def_ins->start_ins >= start_ins && def_ins->end_ins <= end_ins){
                int distance = def_ins - f->def_ins_table.begin();
                gen.insert(distance);
                for(auto tmp = f->def_ins_table.begin();tmp != f->def_ins_table.end();tmp++){
                    //same position, skip
                    if(tmp == def_ins){
                        continue;
                    }
                    if(tmp->def == def_ins->def){
                        kill.insert(tmp - f->def_ins_table.begin());
                    }
                }
                def_ins++;
                if(def_ins == f->def_ins_table.end()){
                    break;
                }
            }
            //更新位向量
            for(auto i : gen){
                bb->gen.set(i);
            }
            for(auto i : kill){
                bb->kill.set(i);
            }
            // if(def_ins == f->def_ins_table.end()){
            //     break;
            // }
        }
    }
}

/* 
 * generate use def bit vector of basic block
 */
void generate_use_def_bit_vec_of_bb(){
    for(auto f = funcs.begin();f != funcs.end();f++){
        auto def_ins = f->def_ins_table.begin();
        for(auto bb = f->bbs.begin();bb != f->bbs.end();bb++){
            bb->use.resize(f->symbol_table.size(),false);
            bb->def.resize(f->symbol_table.size(),false);
            // if(def_ins == f->def_ins_table.end()){
            //     continue;
            // }
            set<int> use;
            set<int> def;
            for(int i=bb->start_ins;i<=bb->end_ins;i++){
                if(def_ins != f->def_ins_table.end() && def_ins->start_ins <= i && def_ins->end_ins >= i){
                    i = def_ins->end_ins;
                    //add use and def
                    for(auto di : def_ins->use){
                        if(def.find(di) == def.end()){
                            use.insert(di);
                        }
                    }
                    if(use.find(def_ins->def) == use.end()){
                        def.insert(def_ins->def);
                    }
                    def_ins++;
                    // if(def_ins == f->def_ins_table.end()){
                    //     break;
                    // }
                }
                else{
                    if(insts[i].op_num == 1){
                        if(ud1_table.at(insts[i].op_code) == USE){
                            upstream_use(use, i, def);
                        }
                    }
                    else if(insts[i].op_num == 2){
                        if(insts[i].op_code != "store" && insts[i].op_code != "move"){
                            upstream_use(use, i, def);
                        }
                    }
                }
            }
            for(auto i : use){
                bb->use.set(i);
                // bb->use.set(f->symbol_table.size()-i-1);
            }
            for(auto i : def){
                bb->def.set(i);
                // bb->def.set(f->symbol_table.size()-i-1);
            }
        }
    }
}

/* 
 * main procedure of data flow analysis of reaching definitions 
 */
void dfa_reaching_definitions(){
    for(auto f = funcs.begin();f != funcs.end();f++){
        int bit_vec_width = f->def_ins_table.size();
        vector< boost::dynamic_bitset<> > out(f->bbs.size());
        vector< boost::dynamic_bitset<> > in(f->bbs.size());

        //in out initialization
        for(auto i = out.begin();i != out.end();i++){
            i->resize(bit_vec_width,false);
        }
        for(auto i = in.begin();i != in.end();i++){
            i->resize(bit_vec_width,false);
        }
        bool flag;
        do{
            flag = false;
            for(int i=0;i<f->bbs.size();i++){
                if(f->bbs[i].pre.size() == 0){
                    in[i].reset();
                }
                else{
                    // IN[B] = UNION{P,pre(B)}OUT[P]
                    in[i] = out[f->bbs[i].pre[0]];
                    for(int j = 1;j < f->bbs[i].pre.size();j++){
                        in[i] |= out[f->bbs[i].pre[j]];
                    }
                }
                boost::dynamic_bitset<> tmp(out[i]);

                // OUT[B] = GEN{B} UNION (IN[B] - KILL{B})
                out[i] = f->bbs[i].gen | (in[i] & ~(f->bbs[i].kill));

                if(out[i] != tmp){
                    flag = true;
                }
            }
        }
        while(flag);

        for(int i=0;i<f->bbs.size();i++){
            f->bbs[i].in = in[i];
            f->bbs[i].out = out[i];
        }
    }
}
/* 
 * main procedure of data flow analysis of living variables
 */
void dfa_living_variables(){
    for(auto f = funcs.begin();f != funcs.end();f++){
        int bit_vec_width = f->symbol_table.size();
        vector< boost::dynamic_bitset<> > out(f->bbs.size());
        vector< boost::dynamic_bitset<> > in(f->bbs.size());

        //in out initialization
        for(auto i = out.begin();i != out.end();i++){
            i->resize(bit_vec_width,false);
        }
        for(auto i = in.begin();i != in.end();i++){
            i->resize(bit_vec_width,false);
        }
        bool flag;
        do{
            flag = false;
            for(int i=f->bbs.size()-1;i>=0;i--){
                if(f->bbs[i].suc.size() == 0){
                    // TODO 默认程序出口处全部变量有效
                    // out[i].reset();
                    out[i].set();
                }
                else{
                    // OUT[B] = UNION{S,suc(B)}OUT[S]
                    out[i] = in[f->bbs[i].suc[0]];
                    for(int j = 1;j < f->bbs[i].suc.size();j++){
                        out[i] |= in[f->bbs[i].suc[j]];
                    }
                }
                boost::dynamic_bitset<> tmp(in[i]);

                // IN[B] = USE{B} UNION (OUT[B] - DEF{B})
                in[i] = f->bbs[i].use | (out[i] & ~(f->bbs[i].def));

                if(in[i] != tmp){
                    flag = true;
                }
            }
        }
        while(flag);

        for(int i=0;i<f->bbs.size();i++){
            f->bbs[i].in = in[i];
            f->bbs[i].out = out[i];
        }
    }

}

/* 
 * main procedure of contstant propagation 
 */
void contstant_propagation(){
    // for all in of a bb, if x has only one def & it is a constant, or x has multiable def & all these values are the same, then use of x can be replaced by that constant

    for(auto f = funcs.begin();f != funcs.end();f++){
        for(auto bb = f->bbs.begin();bb != f->bbs.end(); bb++){
            //first index represent symbol
            //second index represent def_instruction
            vector< vector<int> > def_list(f->symbol_table.size());
            for(int i=0;i<bb->in.size();i++){
                if(bb->in.test(i)){
                    // def_list.push_back(i);
                    def_list[f->def_ins_table[i].def].push_back(i);
                }
            }
            for(int i=0;i<def_list.size();i++){
                bool flag = false;
                long long value;
                if(def_list[i].size() > 0){
                    if(f->def_ins_table[def_list[i][0]].use.size() == 0){
                        flag = true;
                    }
                    value = f->def_ins_table[def_list[i][0]].value;
                    for(int j=0;j<def_list[i].size();j++){
                        if(f->def_ins_table[def_list[i][j]].use.size() != 0 ||value != f->def_ins_table[def_list[i][j]].value){
                            flag = false;
                            break;
                        }   
                    }
                }
                if(flag){
                    // use of symbol `symbol_table[i]` inside `bb` can be replaced by `value`
                    for(int j=bb->start_ins;j<=bb->end_ins;j++){
                        if(insts[j].use1 == i && insts[j].op2 != "GP"){
                            insts[j].op1 = to_string(value);
                        }
                        if(insts[j].use2 == i){
                            insts[j].op2 = to_string(value);
                        }
                    }
                }
            }
        }
    }

}

/* 
 * main procedure of dead code elimination 
 */
void dead_code_elimination(){
    // for all out of a bb, if x is not alive, then def of x inside bb can be eliminated

    for(auto f = funcs.begin();f != funcs.end();f++){
        auto def_ins = f->def_ins_table.begin();

        for(auto bb = f->bbs.begin();bb != f->bbs.end(); bb++){
            if(def_ins == f->def_ins_table.end()){
                break;
            }
            while(def_ins->end_ins <= bb->start_ins){
                def_ins++;
                if(def_ins == f->def_ins_table.end()){
                    break;
                }
            }
            for(int i=0;i<bb->out.size();i++){
                if(!bb->out.test(i)){
                    for(auto def_ins_head = def_ins;def_ins_head->end_ins <= bb->end_ins && def_ins_head != f->def_ins_table.end(); def_ins_head++){
                        //eliminate
                        if(def_ins_head->def == i){
                            for(int j=def_ins_head->start_ins;j<= def_ins_head->end_ins;j++){
                                insts[j].op_code = "nop";
                                insts[j].op_num = 0;
                                insts[j].use1 = -1;
                                insts[j].use2 = -1;
                                insts[j].def = -1;
                            }
                        }
                    }       
                }
            }
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

    construct_symbol_table();
    print_symbol_table();

    //reaching defination
    construct_def_ins_table();
    print_def_ins_table();

    generate_gen_kill_bit_vec_of_bb();
    print_bit_vector(0);

    dfa_reaching_definitions();
    print_in_out();

    contstant_propagation();
    print_optimized_3addr();

    //living variables
    generate_use_def_bit_vec_of_bb();
    print_bit_vector(1);

    dfa_living_variables();
    print_in_out();

    dead_code_elimination();
    print_optimized_3addr();
}