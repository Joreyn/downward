#include "pattern_database.h"

#include "pattern_database_bdd.h"
#include "pattern_database_evmdd.h"
#include "pattern_database_explicit.h"

#include "../utils/logging.h"

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
        shared_ptr<PatternDatabaseExplicit> pdb = make_shared<PatternDatabaseExplicit>(task_proxy, pattern, operator_costs, compute_plan, rng, compute_wildcard_plan);
        return pdb;
    } else if (pdb_type == PDBType::BDD) {
        shared_ptr<PatternDatabaseBDD> pdb = make_shared<PatternDatabaseBDD>(task_proxy, pattern,sym_variables, operator_costs, compute_plan, rng, compute_wildcard_plan);
        return pdb;
    } else {
        return make_shared<PatternDatabaseEVMDD>(task_proxy, pattern, operator_costs, compute_plan, rng, compute_wildcard_plan);
    }
}
}
