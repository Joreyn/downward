#include "pattern_database_bdd.h"

#include <queue>
#include <utility>
#include <limits>
#include "../utils/logging.h"

using namespace std;
using namespace transition;
using namespace utils;

namespace pdbs {
    PatternDatabaseBDD::PatternDatabaseBDD(
            const TaskProxy &task_proxy,
            const Pattern &pattern,
            const vector<int> &operator_costs,
            bool compute_plan,
            const shared_ptr<utils::RandomNumberGenerator> &rng,
            bool compute_wildcard_plan) {
        transition_relation = new TransitionRelation(task_proxy);
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
            //g_log << "in while loop" << endl;
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
        /*
        g_log<<"printing result"<<endl;
        for (const Transition& tr : result){
            g_log<<tr.cost<<" "<<tr.bdd<<endl;
        }
         */

        /* Iteratively remove previous sets from higher cost sets, leading to disjunction
        for (unsigned int i = result.size()-1; i>0;i--){
            result[i].bdd=result[i].bdd*!result[i-1].bdd;
        }*/
        // TODO: then Convert these into a single ADD containing the respective costs on its leaf nodes,
        // implement get_value, then test and add patterns
        cost_map=mgr->constant(numeric_limits<double>::max());
        ADD temp;
        ADD constant;
        for (const Transition& trans : result){
            constant = mgr->constant(trans.cost);
            temp = trans.bdd.Add().Times(constant);
            cost_map.Minimum(temp);
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
        //TODO understand temp.ExistAbstract() and Constrain  and evaluate whether they would be faster
        /*BDD temp=mgr->bddZero();
        for (unsigned int var_id = 0; var_id<transition_relation->preconditions_vector.size(); var_id++){
            for (unsigned int val = 0; val<transition_relation->preconditions_vector[var_id].size(); val++){
                if((_reached.bdd)*transition_relation->effects_vector[var_id][val]!=false) {
                    temp+=transition_relation->preconditions_vector[var_id][val];
                    g_log<<transition_relation->preconditions_vector[var_id][val]<<endl;
                    g_log<<temp<<endl;
                }
            }
        }*/
        for (const BDD& bdd : transition_relation->primed_bdd_variables){
                _reached.bdd=_reached.bdd.ExistAbstract(bdd);
        }
        return _reached;
    }

//TODO delete
    int PatternDatabaseBDD::get_value_bdd(const vector<int> &state) const {
        //g_log << "get_value" << endl;
        //g_log<<state<<endl;
        BDD state_as_bdd = mgr->bddOne();
        for (unsigned int var = 0; var < state.size();var++){
            state_as_bdd*=transition_relation->preconditions_vector[var][state[var]];
        }
        //g_log<<"state as bdd"<<endl;
        //g_log << state_as_bdd << endl;
        //must be ordered by cost
        for(const Transition& res : result){
            if((state_as_bdd * res.bdd != mgr->bddZero())){
                //g_log << "returning cost: "<< res.cost << endl;
                return res.cost;
            }
        }
        //g_log << "returning cost: "<< "infinity" << endl;
        return numeric_limits<int>::max();
    }

    //TODO implement with ADD
    int PatternDatabaseBDD::get_value(const vector<int> &state) const{
        return get_value_bdd(state);
    }

    const Pattern &PatternDatabaseBDD::get_pattern() const {

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
