#ifndef FAST_DOWNWARD_TRANSITION_RELATION_H
#define FAST_DOWNWARD_TRANSITION_RELATION_H

#include <cuddObj.hh>
#include "../task_proxy.h"
#include <map>
#include <utility>
namespace transitionRelation {
    static Cudd * mgr;
    static std::map<int, int> index_map; //usage: index_map[variable_id] to get first index in new domain
    static std::vector<BDD> bdd_variables; //TODO: initialize size at beginning for better runtime
    static std::vector <BDD> goal_bdd; //TODO don't use vector (maybe optional)
    static std::vector <ADD> goal_add;
    static std::vector <ADD> all_reached;

    void init_transRelations(const TaskProxy &task_proxy);
    //needed for initializing Cudd
    int calc_numBDDVars(TaskProxy &task_proxy);

    void init(TaskProxy &task_proxy);

    BDD convert_fact_to_bdd(FactProxy fact, bool is_effect);

    void init_index_map(const TaskProxy &task_proxy);

    void create_goal(const GoalsProxy& gp);



    std::vector<bool> convert_to_bool_vector(int value, int vector_size);

    void create_bddpdb(TaskProxy task_proxy);

    class Trans{
    public:
        BDD bdd;
        int cost;
        std::string name;

        Trans(const BDD& bdd, int cost, std::string name){
            this->bdd=bdd;
            this->cost=cost;
            this->name=std::move(name);
        }
    };
    static std::vector<Trans> transRelations;

    ADD apply(ADD &add, ADD add1, std::vector<transitionRelation::Trans> vector1);

    int get_value(const std::vector<int> &vector);
}

#endif
