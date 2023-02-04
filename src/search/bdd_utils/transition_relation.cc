#include "transition_relation.h"

#include "../utils/logging.h"

using namespace symbolic;
using namespace utils;
namespace transition {
    TransitionRelation::TransitionRelation(const TaskProxy &task_proxy) {
        SymVariables symVariables = SymVariables(task_proxy);

        mgr = symVariables.getMgr();
        bdd_variables = symVariables.getVariables();
        effects_vector = symVariables.getEffectsVector();
        preconditions_vector = symVariables.getPreconditionsVector();
        biimplication = symVariables.getBiimplication();

        OperatorsProxy operators = task_proxy.get_operators();
        //TODO: allocate memory for transRelations here for more speed
        map<int, unsigned int> cost_pos;
        BDD temp;
        //TODO: test, whether this works as intended with not unit-cost operations
        vector<int> zero_vector(task_proxy.get_variables().size());
        vector<int> effect_plus_precondition(task_proxy.get_variables().size());
        for (OperatorProxy o: operators) {
            temp = mgr->bddOne();
            effect_plus_precondition = zero_vector;
            for (EffectProxy eff: o.get_effects()) {
                temp *= fact_to_bdd(eff.get_fact(), true);
                effect_plus_precondition[eff.get_fact().get_variable().get_id()]++;
            }
            for (FactProxy prec: o.get_preconditions()) {
                temp *= fact_to_bdd(prec, false);
                effect_plus_precondition[prec.get_variable().get_id()]++;
            }
            //add biimplication if needed
            for (unsigned int i = 0; i < effect_plus_precondition.size(); i++) {
                if (effect_plus_precondition[i] == 2) {
                } else if (effect_plus_precondition[i] > 2 || effect_plus_precondition[i] < 0) {
                    //TODO edit message, before turning in
                    g_log << "something terrible has happened" << endl;
                } else {
                    temp *= biimplication[i];
                }
            }
            //add the associated BDD to the BDD in the vector, which has the same cost
            int cost = o.get_cost();
            if (cost_pos.count(cost)) {
                transitions[cost_pos[cost]].bdd *= temp;
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
}