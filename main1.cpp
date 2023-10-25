#include "includes/ld_opps.h"
#include <iostream>
using namespace std;

void test_1() {
    struct emp_t {
        char emp_name[30];
        unsigned int emp_id;
        unsigned int age;
        struct emp_t *mgr;
        float salary;
    };
    struct node_t {
        node_t *next;
        int *data;
    };
    field_info_t infos[] = {
        FIELD_INFO(emp_t, emp_id, CHAR, 0), 
        FIELD_INFO(emp_t, age, data_type_t::UINT8, 0),
        FIELD_INFO(emp_t, emp_name, data_type_t::CHAR, 0),
        FIELD_INFO(emp_t, mgr, data_type_t::OBJ_PTR, emp_t),
        FIELD_INFO(emp_t, salary, data_type_t::FLOAT, 0)
    };
    field_info_t infos1[] = {
        FIELD_INFO(node_t, next, data_type_t::OBJ_PTR, node_t),
        FIELD_INFO(node_t, data, data_type_t::OBJ_PTR, 0)
    };
    memory_leak_algorithm_graph leak_detect(new console_printer());
    leak_detect.register_struct("emp_t", sizeof(emp_t), infos, 
                sizeof(infos) / sizeof(field_info_t));
    leak_detect.register_struct("node_t", sizeof(node_t), infos1, 
                    sizeof(infos1) / sizeof(field_info_t));
    leak_detect.print_structure_db();

    // Making some objects
    node_t *node1 = (node_t*) leak_detect.xcalloc("node_t", 1);
    node_t *node2 = (node_t*) leak_detect.xcalloc("node_t", 1);
    node1->data = (int*) leak_detect.xcalloc("int", 1);
    node2->data = (int*) leak_detect.xcalloc("int", 1);

    // node1->next = node2;

    leak_detect.set_object_as_global_root(node1);
    leak_detect.print_object_db();

    // Run leak algorithm
    leak_detect.run();
    leak_detect.report_leaked_objects();
}

int main() {
    test_1();
}

/*
Build scripts :-
g++ -g -c src/ld_opps.cpp -o bin/ld_opps.o
g++ -g -c main1.cpp -o bin/main1.o
g++ -g -o bin/exe_opps bin/ld_opps.o bin/main1.o
*/