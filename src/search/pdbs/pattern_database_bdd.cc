#include "pattern_database_bdd.h"

#include <queue>
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
        Transition goal = Transition(mgr->bddOne(), 0);
        for (const FactProxy &fp: task_proxy.get_goals()) {
            goal.bdd *= transition_relation->fact_to_bdd(fp, false);
        }
        vector<Transition> reached_with_cost;
        vector<unsigned int> index_no_cost;
        vector<unsigned int> index_with_cost;
        for (unsigned int i = 0; i < transition_relation->transitions.size(); i++) {
            if (transition_relation->transitions[i].cost!=0) {
                index_with_cost.push_back(i);
            } else {
                index_no_cost.push_back(i);
            }
        }

        //TODO use pointers/initialize variables earlier
        vector<Transition> result;
        priority_queue<Transition> pq;
        pq.push(goal);
        while (!pq.empty()) {
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
                for (unsigned int i: index_no_cost) {
                    top = apply(transition_relation->transitions[i], top);
                }
                if ((top.bdd * !currently_evaluated.bdd) == mgr->bddZero()) break;
            }
            result.push_back(currently_evaluated);
            for (unsigned int i: index_with_cost) {
                top = currently_evaluated;
                top = apply(transition_relation->transitions[i], top);
                if ((top.bdd * !currently_evaluated.bdd) != mgr->bddZero()){
                    pq.push(top);
                }
            }
        }
        g_log << "pdb created" << endl;
    }


        //TODO: rename function, as the "Vereinigung" is also included here
    Transition PatternDatabaseBDD::apply(const Transition &transition, Transition reached) {
        Transition _reached = reached;

        return Transition(mgr->bddZero(), 0);
    }

//TODO
    int PatternDatabaseBDD::get_value(const vector<int> &state) const {
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
