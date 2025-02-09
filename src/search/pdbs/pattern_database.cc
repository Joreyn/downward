#include "pattern_database.h"

#include "pattern_database_bdd.h"
#include "pattern_database_evmdd.h"
#include "pattern_database_explicit.h"

using namespace std;

namespace pdbs {
shared_ptr<PatternDatabase> compute_pdb(
    PDBType pdb_type,
    const TaskProxy &task_proxy,
    const Pattern &pattern,
    symbolic::SymVariables* sym_variables,
    const vector<int> &operator_costs,
    bool compute_plan,
    const shared_ptr<utils::RandomNumberGenerator> &rng,
    bool compute_wildcard_plan) {
    if (pdb_type == PDBType::Explicit) {
        return make_shared<PatternDatabaseExplicit>(task_proxy, pattern, operator_costs, compute_plan, rng, compute_wildcard_plan);
    } else if (pdb_type == PDBType::BDD) {
        return make_shared<PatternDatabaseBDD>(task_proxy, pattern,sym_variables, operator_costs, compute_plan, rng, compute_wildcard_plan);
    } else {
        return make_shared<PatternDatabaseEVMDD>(task_proxy, pattern, operator_costs, compute_plan, rng, compute_wildcard_plan);
    }
}
}
