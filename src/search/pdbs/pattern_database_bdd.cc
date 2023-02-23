#include "pattern_database_bdd.h"

#include <queue>
#include <limits>
#include <cmath>
#include "../utils/logging.h"
#include "../task_utils/task_properties.h"

using namespace std;
using namespace transition;
using namespace utils;

namespace pdbs {
    PatternDatabaseBDD::PatternDatabaseBDD(
            const TaskProxy &task_proxy,
            const Pattern &pattern,
            symbolic::SymVariables* symVariables,
            const vector<int> &operator_costs,
            bool compute_plan,
            const shared_ptr<utils::RandomNumberGenerator> &rng,
            bool compute_wildcard_plan)
    :pattern(pattern){
        task_properties::verify_no_axioms(task_proxy);
        task_properties::verify_no_conditional_effects(task_proxy);
        assert(operator_costs.empty() ||
               operator_costs.size() == task_proxy.get_operators().size());
        assert(utils::is_sorted_unique(pattern));
        assert(compute_plan==false);
        assert(compute_wildcard_plan==false);
        for (unsigned int var_id = 0; var_id<task_proxy.get_variables().size();var_id++){
            domain_sizes_bdd.push_back(ceil(log2(task_proxy.get_variables()[var_id].get_domain_size())));
        }


        transition_relation = new TransitionRelation(task_proxy, (vector<int>) pattern, symVariables);
        g_log << "TransitionRelation finished" << endl;

        mgr = transition_relation->mgr;

        // calc bdd of goal
        Transition goal = Transition(mgr->bddOne(), 0);
        for (const FactProxy &fp: task_proxy.get_goals()) {
            goal.bdd *= transition_relation->fact_to_bdd(fp, false);
        }

        // TODO: (after merging same cost Transitions)
        //  change index no cost to int, as there will be only one after merging same cost transitions

        // Extract the index of zero cost Transition from all defined transitions
        vector<unsigned int> index_no_cost;
        vector<unsigned int> index_with_cost;
        for (unsigned int i = 0; i < transition_relation->transitions.size(); i++) {
            if (transition_relation->transitions[i].cost!=0) {
                index_with_cost.push_back(i);
            } else {
                index_no_cost.push_back(i);
            }
        }

        //TODO use pointers/initialize variables earlier (result only as local variable here)
        priority_queue<Transition> pq;
        pq.push(goal);
        //TODO edit break condition
        while (!pq.empty()) {
            // TODO: initialize variables earlier, despite warning
            Transition top = pq.top();
            Transition currently_evaluated =pq.top();
            pq.pop();

            //make a single bdd out of all the states with the same cost
            while (!pq.empty() && (pq.top().cost==currently_evaluated.cost)){
                currently_evaluated.bdd+=pq.top().bdd;
                pq.pop();
            }
            //evaluate the 0 cost transitions first
            // (it is also directly added for the not zero cost transition relations)
            while (true) {
                //TODO: since operations with same cost are already merged, replace loop with "if"
                for (unsigned int i: index_no_cost) {
                    currently_evaluated = top;
                    top = apply(transition_relation->transitions[i], top);
                }
                //TODO use LEQ or similar instead of checking ...==mgr->bddZero()
                if ((top.bdd * !currently_evaluated.bdd) == mgr->bddZero()) break;
            }
            for (unsigned int i: index_with_cost) {
                top = currently_evaluated;
                top = apply(transition_relation->transitions[i], top);
                if ((top.bdd * !currently_evaluated.bdd) != mgr->bddZero()){
                    pq.push(top);
                }
            }
            result.push_back(currently_evaluated);
        }

        /* Iteratively remove previous sets from higher cost sets, leading to disjunction
        for (unsigned int i = result.size()-1; i>0;i--){
            result[i].bdd=result[i].bdd*!result[i-1].bdd;
        }*/
        g_log << "creating ADD" << endl;
        DdNode* temp;

        ADD cost;
        ADD constant_max = mgr->constant(numeric_limits<int>::max());
        cost_map_dd=constant_max.getNode();
        cost_map_add=constant_max;
        ADD temp_add;
        Cudd_Ref(cost_map_dd);
        g_log<<result.size()<<endl;
        for (const Transition& trans : result){
            cost=mgr->constant(trans.cost);
            temp_add=trans.bdd.Add().Ite(cost,constant_max);
            cost_map_add=cost_map_add.Minimum(temp_add);


            temp = Cudd_addIte(mgr->getManager(),trans.bdd.Add().getNode(),mgr->constant(trans.cost).getNode(),constant_max.getNode());
            Cudd_Ref(temp);
            cost_map_dd=Cudd_addApply(mgr->getManager(), Cudd_addMinimum, cost_map_dd, temp);
            Cudd_RecursiveDeref(mgr->getManager(), temp);
        }
        g_log << "pdb created" << endl;
    }


        //TODO: rename function, as the conjunction is also included here
    Transition PatternDatabaseBDD::apply(const Transition &transition, const Transition& reached) {
        //g_log << "apply" << endl;
        Transition _reached = reached;
        //TODO understand _reached.bdd.AdjPermuteX() and evaluate whether it would be faster
        _reached.bdd=_reached.bdd.SwapVariables(transition_relation->primed_bdd_variables,
                                                transition_relation->bdd_variables);
        _reached.bdd*=transition.bdd;
        _reached = forget(_reached);

        if (transition.cost==0){
            g_log<<"cost zero transition"<<endl;
            return Transition(_reached.bdd+=reached.bdd, _reached.cost+transition.cost);
        }
        //TODO: evaluate, whether to use _reached.bdd+=reached.bdd or _reached.bdd this is helpful
        return Transition(_reached.bdd+=reached.bdd, _reached.cost+transition.cost);
    }

    inline Transition &PatternDatabaseBDD::forget(Transition &_reached) {
        //TODO understand temp.ExistAbstract() and Constrain and evaluate whether they would be faster
        for (const BDD& bdd : transition_relation->primed_bdd_variables){
                _reached.bdd=_reached.bdd.ExistAbstract(bdd);
        }
        return _reached;
    }

    //TODO delete
    int PatternDatabaseBDD::get_value_bdd(const vector<int> &state) const {
        BDD state_as_bdd = mgr->bddOne();
        for (unsigned int var = 0; var < state.size();var++){
            state_as_bdd*=transition_relation->preconditions_vector[var][state[var]];
        }
        //must be ordered by cost
        for(const Transition& res : result){
            if((state_as_bdd * res.bdd != mgr->bddZero())){
                return res.cost;
            }
        }
        return numeric_limits<int>::max();
    }

    //TODO implement with ADD
    int PatternDatabaseBDD::get_value(const vector<int> &state) const{
        vector<int> converted;
        int value;
        for (unsigned int var_id=0;var_id<state.size();var_id++){
            value = state[var_id];
            for(int i = 0; i < domain_sizes_bdd[var_id];i++){
                converted.push_back(value & 1); //unprimed
                converted.push_back(value & 1); //primed
                value = value >> 1;
            }
        }
        DdNode* eval_dd = Cudd_Eval(mgr->getManager(), cost_map_dd, converted.data());
        int placeholder = (int) Cudd_V(eval_dd);

        ADD eval_add = cost_map_add.Eval(converted.data());
        int placeholder_add= (int) Cudd_V(eval_add.getNode());

        int res = get_value_bdd(state);

        if(res!=placeholder||res!=placeholder_add||res!=(int) Cudd_V(eval_add.getRegularNode())) {
            g_log <<endl<< "BDD_value: " << res <<
                  "DD_value: " << placeholder <<
                  "ADD_value(CudV)" << placeholder_add <<
                  "ADD_value(direct)" << (int) Cudd_V(eval_add.getRegularNode()) << endl <<
                  "ADD eval input: " << converted << endl << endl;
        }
        return get_value_bdd(state);
    }

    const Pattern &PatternDatabaseBDD::get_pattern() const {
        return pattern;
    }

    int PatternDatabaseBDD::get_size() const {
    }

    vector<vector<OperatorID>> &&PatternDatabaseBDD::extract_wildcard_plan() {

    }

    double PatternDatabaseBDD::compute_mean_finite_h() const {

    };

    bool PatternDatabaseBDD::is_operator_relevant(const OperatorProxy &op) const {

    }
}
