#include "pattern_database_bdd.h"

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
    bool compute_wildcard_plan){
    transition_relation = new TransitionRelation(task_proxy);
    g_log << "TransitionRelation finished" << endl;
    mgr=transition_relation->mgr;
    Transition goal = Transition(mgr->bddOne(), 0);
    for (const FactProxy& fp : task_proxy.get_goals()){
        goal.bdd*=transition_relation->fact_to_bdd(fp, false);
    }
    vector<Transition> reached_with_cost;
    reached_with_cost.push_back(goal);


    bool change_occurred=false;
    //TODO: make more efficient
    /*do{
        for (const Transition& trans : transition_relation->transitions){
            Transition temp = apply(trans, goal);
        }

    }while(change_occurred);*/


    g_log << "pdb created" << endl;
}

    Transition PatternDatabaseBDD::apply(const Transition &transition, Transition goal) {
    /*if (old!=(old*=transition)){
        change_occurred = true;
    }else{}*/
        return Transition(BDD(), 0);
    }

//TODO
int PatternDatabaseBDD::get_value(const vector<int> &state) const {
}

const Pattern &PatternDatabaseBDD::get_pattern() const {

}

int PatternDatabaseBDD::get_size() const {

}

vector<vector<OperatorID>> && PatternDatabaseBDD::extract_wildcard_plan() {

}

double PatternDatabaseBDD::compute_mean_finite_h() const {

};

bool PatternDatabaseBDD::is_operator_relevant(const OperatorProxy &op) const {

}




}
