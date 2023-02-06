#ifndef FAST_DOWNWARD_SYM_VARIABLES_H
#define FAST_DOWNWARD_SYM_VARIABLES_H

#include <cuddObj.hh>
#include "../task_proxy.h"
#include <map>


using namespace std;
namespace symbolic {
    class SymVariables {
        //TODO make some variables static, and only initialize if not already (for multiple Patterns)
        Cudd * mgr;
        vector <BDD> bdd_variables, primed_bdd_variables;
    public:
        const vector<BDD> &getPrimedBddVariables() const;

    private:
        /* TODO: technically it is better to use a index map,
         such that preconditions/effects vector has smaller size (because of pattern),
         but (once implemented), all bdd_pdb can share these vectors, leading to possibly slightly higher
         */
        map<int,int> index_map; //TODO probably not needed, make local
        vector<vector<BDD>> preconditions_vector, effects_vector; //_[var_id][val]
        vector<BDD> biimplication;
    public:
        const vector<BDD> &getBiimplication() const;

    private:
        //biimplication[var_id]= bdd, for which var.val=var'.val
        //TODO: implement restricted BDD variables
        vector<vector<BDD>> preconditions_vector_restricted, effects_vector_restricted; //_[var_id][val]
        int calc_numBDDVars(const TaskProxy &task_proxy);
        vector<bool> convert_to_bool_vector(int value, int vector_size);
        void create_index_map(const TaskProxy &task_proxy);

    public:
        explicit SymVariables(const TaskProxy &task_proxy); //TODO understand explicit
        virtual ~SymVariables();
        void init_prec_eff(const TaskProxy &task_proxy);

        Cudd *getMgr() const;

        const vector<BDD> &getVariables() const;

        const vector<vector<BDD>> &getPreconditionsVector() const;

        const vector<vector<BDD>> &getEffectsVector() const;

    };
}

#endif //FAST_DOWNWARD_SYM_VARIABLES_H
