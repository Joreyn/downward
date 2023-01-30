#ifndef PDBS_PATTERN_DATABASE_BDD_H
#define PDBS_PATTERN_DATABASE_BDD_H

#include "pattern_database.h"
#include "../task_utils/transition_relation.h"


namespace pdbs {
class PatternDatabaseBDD : public PatternDatabase {
    ADD all_reached;
public:
    ADD models(const GoalsProxy& gp);
    ADD apply(ADD &add, ADD add1, std::vector<transitionRelation::Trans> vector1);
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
    void create_bddpdb(TaskProxy task_proxy);



};
}

#endif
