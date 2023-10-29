#include "includes/ld_opps.h"
#include <iostream>
using namespace std;

/*
    Why java have garbage collection but C/C++ does not ?
        * Java in interpreted. That means there is a process (JVM) which is monitoring our process. 
            Hence it is easier to count references to a memory location and detect when the reference count
            becomes zero. 
        * C/C++ run as machine executable without any monitoring. Hence it is very difficult (close to impossible)
            to maintain the reference count of a memory location. 
        * In java, there are no pointers but references and object can only be created in heap. Hence it is easier 
            to decide that a particular reference is pointing to a heap memory location. 
        * In C/C++, there are pointers which can point to almost any memory section (heap, stack, data, bss etc). Not very easy
            to decide whether a pointer is pointing to heap section or not. 
    This leak library has many limitations :-
        * Storing pointer of an object to a int variable (refer limitation_1()).
        * When a pointer in a structure member points to a non-heap memory location. 
        *   Java does not allow this. 
        * Embedded objects (refer limitation_2()). We cannot have embedded objects in java. 
        *   When we embed objects in java, we are actually storing its reference. 
        * No support for unions and classes. Very difficult to handle them. 
        * Does not consider objects which are created but no reference by any global object. 
        *   Here our tool will report leak but it might not be a leak. 
        * If the struct members are private or protected, then it is not possible to access them by our tool. 

    TODO :-
        * Build using makefile. Build as static library. 
        * Add simple documentation. 
*/

void limitation_1() {
    struct emp_t {
        char name[32];
        unsigned int designation;
    };

    struct des_t {
        char name[32];
        int job_code;
    };

    emp_t *emp1 = (emp_t*) calloc(1, sizeof(emp_t));
    des_t *des1 = (des_t*) calloc(1, sizeof(des_t));
    // Storing reference in unsigned int. C/C++ allows it. Java doesn't. 
    emp1->designation = (unsigned long long)des1;
    // We can get back the object like this. 
    des_t *designation = (des_t*)emp1->designation;
    // Here there will be no leak but our tool will detect leak. 
}

void limitation_2() {
    struct des_t {
        char name[32];
    };
    struct emp_t {
        char name[32];
        // This will store the complete struct. 
        struct des_t des;
    };
    des_t *des = (des_t*) calloc(1, sizeof(des_t));
    emp_t *emp = (emp_t*) calloc(1, sizeof(emp_t));
    // Storing the complete object. Not pointer. 
    // There will be no leak but our tool will report leak. 
    emp->des = *des;
}

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