#include "transition_relation.h"

#include "math.h"
#include "../utils/logging.h"

using namespace std;


namespace transitionRelation {
//TODO test
void init(TaskProxy &task_proxy){
    utils::g_log<<"is initailizing transrelations= "<<std::endl;
    //initialize CUDD
    //TODO: allocate memory for CUDD (cache size etc. (possibly just copy from other file))
    int numBDDVars=calc_numBDDVars(task_proxy);
    //place mgr to another function(cudd_init or something) as this works only for binary variables + cleaner that way
    mgr = new Cudd (numBDDVars);
    for (int i = 0; i<numBDDVars;i++){
        bdd_variables.push_back(mgr->bddVar(i));
    }

    //Initialize the index_map
    init_index_map(task_proxy);

    //TODO delete
    utils::g_log << "printing index map:" <<std::endl;
    for (auto & it : index_map){
        utils::g_log << it.first << '\t' << it.second << std::endl;
    }

    //Create a transition relation for all operators and save them in vector transRelations
    utils::g_log << "init transRelations:" << std::endl;
    init_transRelations(task_proxy);

    //TODO delete
    utils::g_log << "printing transRelations:" <<std::endl;
    for (auto & it : transRelations){
        utils::g_log <<it.name <<std::endl << it.cost << std::endl << it.bdd << std::endl;
    }

    create_goal(task_proxy.get_goals());

    create_bddpdb(task_proxy);
}

//TODO test
void init_index_map(const TaskProxy &task_proxy) {
    int current_index=0;
    int size;
    VariablesProxy variables = task_proxy.get_variables();
    for (VariableProxy variable:variables){
        index_map[variable.get_id()]=current_index;
        size=ceil(log2(variable.get_domain_size()));
        current_index+=2*size;
    }
}

//TODO debug
void init_transRelations(const TaskProxy &task_proxy) {
    utils::g_log << "in function transRelations:" <<std::endl;
    OperatorsProxy operators=task_proxy.get_operators();
    //TODO: allocate memory for transRelations here for more speed
    for (OperatorProxy o:operators){
        int cost = o.get_cost();
        BDD test=mgr->bddOne();
        for (EffectProxy eff:o.get_effects()){
            //in symvariables all the bdd are saved in a vector
            // (leading to std::vector<std::vector<int>> bdd_index_pre, bdd_index_eff;)
            utils::g_log << "effect loop:" <<std::endl;
            test*=convert_fact_to_bdd(eff.get_fact(), true);
        }
        for (FactProxy prec :o.get_preconditions()){
            utils::g_log << "precondition loop:" <<std::endl;
            test*=convert_fact_to_bdd(prec, false);
        }
        transRelations.emplace_back(test,cost,o.get_name());
    }
}

//TODO: make more efficient, especially for loop
// (possibly initialize a map, which contains all the val -> [bool], thus the calculation only needed once)
BDD convert_fact_to_bdd(FactProxy fact, bool is_effect){
    //utils::g_log << "function convert_fact_to_bdd:" <<std::endl;
    BDD bdd=mgr->bddOne();
    int index = index_map[fact.get_variable().get_id()];
    if (is_effect){
        index++;
    }
    int j = 0;
    int num_of_bdd_variables=ceil(log2(fact.get_variable().get_domain_size()));

    /*utils::g_log << "before convert_to_bool_vector:" <<
    "domainsize: "<<fact.get_variable().get_domain_size()<<std::endl;
     */
    std::vector<bool> vector = convert_to_bool_vector(fact.get_value(),num_of_bdd_variables);
    /*utils::g_log << "before for loop:" <<std::endl;*/
    for (int i = index; i<num_of_bdd_variables+index; i+=2){
    //TODO make cleaner with inline operator or something
        if (vector[j]){
            bdd*=bdd_variables[i];
        }else{
            bdd*=!bdd_variables[i];
        }
    }
    return bdd;
}

std::vector<bool> convert_to_bool_vector(int value, int vector_size){
    //TODO!!!!!!: debug in this function
    //utils::g_log << "function convert_to_bool_vector:" <<std::endl;
    //utils::g_log << "value: " << value << "vector size:" << vector_size<<std::endl;
    std::vector<bool> bool_vector;
    for (int i = 0; i<vector_size; i++){
        bool_vector.push_back(value&1);
        value = value>>1;
    }

    //TODO delete print statzement
    //utils::g_log << "bool vector:" <<std::endl;
    for (bool b : bool_vector){
        utils::g_log << b ;
    }
    utils::g_log << std::endl;

    return bool_vector;
}

int calc_numBDDVars(TaskProxy &task_proxy){
    int sum = 0;
    for (VariableProxy variable:task_proxy.get_variables()){
        utils::g_log<<"name: "<<variable.get_name()<<std::endl;
        utils::g_log<<"domain size: "<<variable.get_domain_size()<<std::endl;
        utils::g_log<<"bdd variables: "<<ceil(log2(variable.get_domain_size()))<<std::endl;
        sum+=ceil(log2(variable.get_domain_size()));
    }
    utils::g_log<<"numBDDVars= "<<2*sum<<std::endl;
    return 2*sum; //double the sum, because x,x'
}

    void create_goal(const GoalsProxy &gp) {
        utils::g_log << "in create_goal" << endl;
        BDD test = mgr->bddOne(); //TODO leads to null pointer or something
        utils::g_log << "after initializing test" << endl;
        utils::g_log << test << endl;
        for (FactProxy fact : gp){
            test*=convert_fact_to_bdd(fact,false);
            utils::g_log << test << endl;
        }
        utils::g_log << "after for loop" << endl;

        utils::g_log<< "bdd of goal states:" <<endl<<test << endl;
        goal_bdd.push_back(test);
        utils::g_log<< "add of goal states:" <<endl<<test.Add() << endl;
        goal_add.push_back(test.Add());
    }

    void create_bddpdb(TaskProxy task_proxy) {
        std::vector <ADD> reached;
        utils::g_log << "PatternDatabaseBDD::create_bddpdb after transitionRelation::init" << endl;
        utils::g_log << goal_add << endl;
        utils::g_log << goal_bdd << endl;
        ADD goals = transitionRelation::mgr->addZero();
        utils::g_log << goals << endl;
        goals += goal_add[0];
        utils::g_log << goals << endl;
        utils::g_log << "PatternDatabaseBDD::create_bddpdb after getting goals" << endl;
        //TODO crash happens after this line, also look at ADD and whether creation is as expected
        reached.push_back(goals);
        all_reached.push_back(goals);
        int i = 0;
        do{
            reached.push_back(apply(reached[i], all_reached[0], transRelations));
            i++;
        }while (reached[i-1]!=reached[i]);
        //TODO delete print statement
        utils::g_log << "printing reached:" << endl;
        for (ADD add : reached){
            utils::g_log << add << endl;
        }
    }

    int get_value(const std::vector<int> &vector) {
        return 0;
    }

    //TODO implement and rename
    ADD apply(ADD &add, ADD add1, std::vector<transitionRelation::Trans> vector1) {
    return ADD();
}
}