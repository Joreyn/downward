#include "pattern_database_evmdd.h"

using namespace std;

namespace pdbs {
PatternDatabaseEVMDD::PatternDatabaseEVMDD(
    const TaskProxy &task_proxy,
    const Pattern &pattern,
    const vector<int> &operator_costs,
    bool compute_plan,
    const shared_ptr<utils::RandomNumberGenerator> &rng,
    bool compute_wildcard_plan) {

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
