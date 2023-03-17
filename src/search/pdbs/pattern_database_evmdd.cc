#include "pattern_database_evmdd.h"
#include "../utils/logging.h"


using namespace std;

namespace pdbs {
PatternDatabaseEVMDD::PatternDatabaseEVMDD(
    const TaskProxy &task_proxy,
    const Pattern &pattern,
    const vector<int> &operator_costs,
    bool compute_plan,
    const shared_ptr<utils::RandomNumberGenerator> &rng,
    bool compute_wildcard_plan) {

    /*
    MEDDLY::initialize();
    MEDDLY::domain *domain = MEDDLY::createDomain();
    //TODO add policies
    MEDDLY::forest *transitions = domain->createForest(true, MEDDLY::forest::INTEGER, MEDDLY::forest::EVPLUS);
    MEDDLY::dd_edge ddEdge(transitions);


    transitions->evaluate()
    domain->createVariablesBottomUp();
    MEDDLY::createVariable()
    int num_vars = 3;
    MEDDLY::forest f(num_vars);
    MEDDLY::dd_edge evmdd = f.make_dd(MEDDLY::ev);
    MEDDLY::dd_edge e1 = f.insert(0, 1, 1, evmdd);
    MEDDLY::dd_edge e2 = f.insert(1, 2, 1, evmdd);
    MEDDLY::dd_edge e3 = f.insert(2, 0, 0, evmdd);
    evmdd = f.reduce(evmdd);
    f.print_mdd(evmdd, std::cout);*/
}

int PatternDatabaseEVMDD::get_value(const vector<int> &state) const {

}

const Pattern &PatternDatabaseEVMDD::get_pattern() const {

}

int PatternDatabaseEVMDD::get_size() const {

}

vector<vector<OperatorID>> && PatternDatabaseEVMDD::extract_wildcard_plan() {

}

double PatternDatabaseEVMDD::compute_mean_finite_h() const {

};

bool PatternDatabaseEVMDD::is_operator_relevant(const OperatorProxy &op) const {


}
}
