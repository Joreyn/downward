#ifndef FAST_DOWNWARD_TRANSITION_RELATION_H
#define FAST_DOWNWARD_TRANSITION_RELATION_H

#include <cuddObj.hh>
#include "../task_proxy.h"
#include <map>
#include <utility>
#include "sym_variables.h"
namespace transition {
    struct Transition{
        BDD bdd;
        int cost;
        Transition(const BDD& bdd, int cost);
    };
    class TransitionRelation {
    public:
        Cudd * mgr;
        vector <BDD> bdd_variables;
        vector<vector<BDD>> preconditions_vector, effects_vector; //_[var_id][val]
        vector<BDD> biimplication;
        vector<Transition> transitions;
        BDD fact_to_bdd(FactProxy fact_proxy, bool is_effect);
        explicit TransitionRelation(const TaskProxy &task_proxy);
    };
}

#endif //FAST_DOWNWARD_TRANSITION_RELATION_H
