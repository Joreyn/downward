#ifndef FAST_DOWNWARD_TRANSITION_RELATION_H
#define FAST_DOWNWARD_TRANSITION_RELATION_H

#include "../../dd_libs/cudd-3.0.0/cplusplus/cuddObj.hh"
#include "pattern_database.h"
#include <map>
namespace transitionRelation {
    static Cudd * mgr;
    static std::map<int, int> index_map; //usage: index_map[variable_id] to get first index in new domain
    static std::vector<BDD> bdd_variables; //TODO: initialize size at beginning for better runtime

    //needed for initializing Cudd
    int calc_numBDDVars(TaskProxy &task_proxy);

    void init(TaskProxy &task_proxy);

    BDD convert_fact_to_bdd(FactProxy fact, bool is_effect);

    void init_index_map(const TaskProxy &task_proxy);

    void init_transRelations(const TaskProxy &task_proxy);

    std::vector<bool> convert_to_bool_vector(int value, int vector_size);

    class Trans{
        BDD bdd;
        int cost;
        std::string name;
    public:
        Trans(BDD bdd, int cost, std::string name){
            this->bdd=bdd;
            this->cost=cost;
            this->name=name;
        }
    };
}

#endif
