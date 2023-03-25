#ifndef PDBS_PATTERN_DATABASE_H
#define PDBS_PATTERN_DATABASE_H

#include "types.h"

#include "../task_proxy.h"
#include "../bdd_utils/sym_variables.h"

#include <utility>
#include <vector>

namespace utils {
class LogProxy;
class RandomNumberGenerator;
}

namespace pdbs {
class PatternDatabase {
public:
    virtual ~PatternDatabase() = default;
    virtual int get_value(const std::vector<int> &state) const = 0;
    virtual const Pattern &get_pattern() const = 0;
    virtual int get_size() const = 0;
    virtual int get_rel_size() const = 0;
    virtual std::vector<std::vector<OperatorID>> && extract_wildcard_plan() = 0;
    virtual double compute_mean_finite_h() const = 0;
    virtual bool is_operator_relevant(const OperatorProxy &op) const = 0;
};

extern std::shared_ptr<PatternDatabase> compute_pdb(
    PDBType pdb_type,
    const TaskProxy &task_proxy,
    const Pattern &pattern,
    symbolic::SymVariables* sym_variables = nullptr,
    const std::vector<int> &operator_costs = std::vector<int>(),
    bool compute_plan = false,
    const std::shared_ptr<utils::RandomNumberGenerator> &rng = nullptr,
    bool compute_wildcard_plan = false);
}

#endif
