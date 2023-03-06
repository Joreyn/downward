#include "transition_relation.h"

#include "../utils/logging.h"

using namespace symbolic;
using namespace utils;
namespace transition {
    TransitionRelation::TransitionRelation(const TaskProxy &task_proxy, vector<int> pattern, SymVariables* symVariables) {
        //TODO: don't create new SymVariables for same taskProxy
        //SymVariables symVariables = SymVariables(task_proxy);

        mgr = symVariables->getMgr();
        bdd_variables = symVariables->getVariables();
        primed_bdd_variables = symVariables->getPrimedBddVariables();
        effects_vector = symVariables->getEffectsVector();
        preconditions_vector = symVariables->getPreconditionsVector();
        biimplication = symVariables->getBiimplication();

        OperatorsProxy operators = task_proxy.get_operators();
        //TODO: allocate memory for transRelations here for more speed
        map<int, unsigned int> cost_pos;
        BDD temp;

        //TODO: test, whether this works as intended with not unit-cost operations

        // TODO: if the preconditions and effects of a operator are ordered by var_id,
        //  this can be simplified to remove the usage of std::count (possibly priorityqueue)
        //   and it can be be further simplified, if pattern is ordered by var_id
        vector<int> zero_vector(task_proxy.get_variables().size());
        vector<int> effect_plus_precondition(task_proxy.get_variables().size());
        for (OperatorProxy o: operators) {
            temp = mgr->bddOne();
            effect_plus_precondition = zero_vector;
            for (EffectProxy eff: o.get_effects()) {
                if (count(pattern.begin(), pattern.end(), eff.get_fact().get_variable().get_id())) {
                    temp *= fact_to_bdd(eff.get_fact(), true);
                    effect_plus_precondition[eff.get_fact().get_variable().get_id()] = 1;
                }
            }
            for (FactProxy prec: o.get_preconditions()) {
                if (count(pattern.begin(), pattern.end(), prec.get_variable().get_id())) {
                    temp *= fact_to_bdd(prec, false);
                }
            }
            //add biimplication if needed
            for (unsigned int i = 0; i < effect_plus_precondition.size(); i++) {
                if (effect_plus_precondition[i] == 0 &&
                    count(pattern.begin(), pattern.end(), i)) {
                        temp *= biimplication[i];
                } else if (effect_plus_precondition[i] > 1 || effect_plus_precondition[i] < 0) {
                    //TODO remove message, before turning in
                    g_log << "something terrible has happened" << endl;
                }
            }
            //add the associated BDD to the BDD in the vector, which has the same cost
            int cost = o.get_cost();
            if (cost_pos.count(cost)) {
                transitions[cost_pos[cost]].bdd += temp;
            } else {
                transitions.emplace_back(temp, cost);
                cost_pos[cost] = transitions.size() - 1;
            }
        }


    }

    BDD TransitionRelation::fact_to_bdd(FactProxy fact_proxy, bool is_effect) {
        if (is_effect) {
            return effects_vector[fact_proxy.get_variable().get_id()][fact_proxy.get_value()];
        } else {
            return preconditions_vector[fact_proxy.get_variable().get_id()][fact_proxy.get_value()];
        }
    }

    Transition::Transition(const BDD &bdd, int cost) : bdd(bdd), cost(cost) {}

    //TODO reverse here, and change order in calling function
    bool Transition::operator<(const Transition &rhs) const {
        return cost > rhs.cost;
    }
}