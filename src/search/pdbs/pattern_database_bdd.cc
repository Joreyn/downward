#include "pattern_database_bdd.h"

using namespace std;

namespace pdbs {
PatternDatabaseBDD::PatternDatabaseBDD(
    const TaskProxy &task_proxy,
    const Pattern &pattern,
    const vector<int> &operator_costs,
    bool compute_plan,
    const shared_ptr<utils::RandomNumberGenerator> &rng,
    bool compute_wildcard_plan){

    create_bddpdb(task_proxy);
}

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

//TODO complete
void PatternDatabaseBDD::create_bddpdb(TaskProxy task_proxy) {
    std::vector <ADD> reached;
    transitionRelation::init(task_proxy);
    std::cout<<"hi"<<endl;
    reached.push_back(models(task_proxy.get_goals()));
    all_reached=models(task_proxy.get_goals());
    int i = 0;
    do{
        reached.push_back(apply(reached[i], all_reached, transitionRelation::transRelations));
        i++;
    }while (reached[i-1]!=reached[i]);
}

//TODO test function
ADD PatternDatabaseBDD::models(const GoalsProxy& gp) {
BDD test = transitionRelation::mgr->bddOne();
for (FactProxy fact : gp){
    test*=transitionRelation::convert_fact_to_bdd(fact,false);
}
    return test.Add();
}

ADD PatternDatabaseBDD::apply(ADD &add, ADD add1, std::vector<transitionRelation::Trans> vector1) {
    return ADD();
}


}
