#ifndef PDBS_PATTERN_DATABASE_BDD_H
#define PDBS_PATTERN_DATABASE_BDD_H

#include "pattern_database.h"

namespace pdbs {
class PatternDatabaseBDD : public PatternDatabase {
public:
    PatternDatabaseBDD(
        const TaskProxy &task_proxy,
        const Pattern &pattern,
        const std::vector<int> &operator_costs = std::vector<int>(),
        bool compute_plan = false,
        const std::shared_ptr<utils::RandomNumberGenerator> &rng = nullptr,
        bool compute_wildcard_plan = false);
    virtual ~PatternDatabaseBDD() override = default;
    virtual int get_value(const std::vector<int> &state) const override;
    virtual const Pattern &get_pattern() const override;
    virtual int get_size() const override;
    virtual std::vector<std::vector<OperatorID>> && extract_wildcard_plan() override;
    virtual double compute_mean_finite_h() const override;
    virtual bool is_operator_relevant(const OperatorProxy &op) const override;
};
}

#endif
