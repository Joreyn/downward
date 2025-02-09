#include "incremental_canonical_pdbs.h"

#include "canonical_pdbs.h"
#include "pattern_database.h"

#include <limits>

using namespace std;

namespace pdbs {
IncrementalCanonicalPDBs::IncrementalCanonicalPDBs(
    const TaskProxy &task_proxy, PDBType pdb_type, const PatternCollection &intitial_patterns)
    : task_proxy(task_proxy),
      pdb_type(pdb_type),
      patterns(make_shared<PatternCollection>(intitial_patterns.begin(),
                                              intitial_patterns.end())),
      pattern_databases(make_shared<PDBCollection>()),
      pattern_cliques(nullptr),
      size(0) {
    pattern_databases->reserve(patterns->size());
    if((pdb_type==PDBType::BDD)&&(sym_variables== nullptr)){
        //TODO possible debug
        sym_variables= make_shared<symbolic::SymVariables>(task_proxy);
    } else {
        sym_variables = nullptr;
    }

    for (const Pattern &pattern : *patterns)
        add_pdb_for_pattern(pattern);
    are_additive = compute_additive_vars(task_proxy);
    recompute_pattern_cliques();
}

void IncrementalCanonicalPDBs::add_pdb_for_pattern(const Pattern &pattern) {
    pattern_databases->push_back(compute_pdb(pdb_type, task_proxy, pattern, sym_variables.get()));
    size += pattern_databases->back()->get_size();
}

void IncrementalCanonicalPDBs::add_pdb(const shared_ptr<PatternDatabase> &pdb) {
    patterns->push_back(pdb->get_pattern());
    pattern_databases->push_back(pdb);
    size += pattern_databases->back()->get_size();
    recompute_pattern_cliques();
}

void IncrementalCanonicalPDBs::recompute_pattern_cliques() {
    pattern_cliques = compute_pattern_cliques(*patterns,
                                              are_additive);
}

vector<PatternClique> IncrementalCanonicalPDBs::get_pattern_cliques(
    const Pattern &new_pattern) {
    return pdbs::compute_pattern_cliques_with_pattern(
        *patterns, *pattern_cliques, new_pattern, are_additive);
}

int IncrementalCanonicalPDBs::get_value(const State &state) const {
    CanonicalPDBs canonical_pdbs(pattern_databases, pattern_cliques);
    return canonical_pdbs.get_value(state);
}

bool IncrementalCanonicalPDBs::is_dead_end(const State &state) const {
    state.unpack();
    for (const shared_ptr<PatternDatabase> &pdb : *pattern_databases)
        if (pdb->get_value(state.get_unpacked_values()) == numeric_limits<int>::max())
            return true;
    return false;
}

PatternCollectionInformation
IncrementalCanonicalPDBs::get_pattern_collection_information(
    utils::LogProxy &log) const {
    PatternCollectionInformation result(task_proxy, patterns, log);
    result.set_pdbs(pattern_databases);
    result.set_pattern_cliques(pattern_cliques);
    return result;
}
}
