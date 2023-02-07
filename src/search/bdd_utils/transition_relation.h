#ifndef FAST_DOWNWARD_TRANSITION_RELATION_H
#define FAST_DOWNWARD_TRANSITION_RELATION_H

#include <cuddObj.hh>
#include "../task_proxy.h"
#include <map>
#include <utility>
#include "sym_variables.h"
namespace transition {
    //TODO: use Cudd_bddIte
    struct Transition{
        BDD bdd;
        int cost;
        Transition(const BDD& bdd, int cost);
        bool operator<(const Transition &rhs) const;
    };

    class TransitionRelation {
    public:
        Cudd * mgr;
        vector <BDD> bdd_variables, primed_bdd_variables;
        vector<vector<BDD>> preconditions_vector, effects_vector; //_[var_id][val]
        vector<BDD> biimplication;
        vector<Transition> transitions;
        BDD fact_to_bdd(FactProxy fact_proxy, bool is_effect);
        TransitionRelation(const TaskProxy &task_proxy, vector<int> pattern);
    };
}

#endif //FAST_DOWNWARD_TRANSITION_RELATION_H
