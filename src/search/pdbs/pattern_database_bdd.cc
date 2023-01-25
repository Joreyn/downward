#include "pattern_database_bdd.h"

using namespace std;

namespace pdbs {
PatternDatabaseBDD::PatternDatabaseBDD(
    const TaskProxy &task_proxy,
    const Pattern &pattern,
    const vector<int> &operator_costs,
    bool compute_plan,
    const shared_ptr<utils::RandomNumberGenerator> &rng,
    bool compute_wildcard_plan) {
    
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
}
