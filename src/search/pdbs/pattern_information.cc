#include "pattern_information.h"

#include "pattern_database.h"
#include "validation.h"
#include "../utils/logging.h"

#include <cassert>

using namespace std;

namespace pdbs {
PatternInformation::PatternInformation(
    const TaskProxy &task_proxy,
    Pattern pattern,
    utils::LogProxy &log)
    : task_proxy(task_proxy),
      pattern(move(pattern)),
      pdb(nullptr) {
    validate_and_normalize_pattern(task_proxy, this->pattern, log);
}

bool PatternInformation::information_is_valid() const {
    return !pdb || pdb->get_pattern() == pattern;
}

void PatternInformation::create_pdb_if_missing(PDBType pdb_type) {
    if (!pdb) {
        if(pdb_type==PDBType::BDD){
            symbolic::SymVariables sym_variables = symbolic::SymVariables(task_proxy);
            pdb = compute_pdb(pdb_type, task_proxy, pattern, &sym_variables);
        }else if(pdb_type==PDBType::EVMDD){
            //TODO
            pdb = compute_pdb(pdb_type, task_proxy, pattern, nullptr);
        }else if(pdb_type==PDBType::Explicit){
            symbolic::SymVariables sym_variables = symbolic::SymVariables(task_proxy);
            pdb = compute_pdb(pdb_type, task_proxy, pattern, nullptr);
        }

    }
}

void PatternInformation::set_pdb(const shared_ptr<PatternDatabase> &pdb_) {
    pdb = pdb_;
    assert(information_is_valid());
}

const Pattern &PatternInformation::get_pattern() const {
    return pattern;
}

shared_ptr<PatternDatabase> PatternInformation::get_pdb(PDBType pdb_type) {
    create_pdb_if_missing(pdb_type);
    utils::g_log << "PDB: size = "<< pdb->get_size()<<endl;
    return pdb;
}
}
