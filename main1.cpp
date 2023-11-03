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

    Many types of garbage collectors :-
        * Reference counting garbage collector - very difficult to implement for fully compiled languages. 
        * Tracing garbage collectors - same root object concept (mark and sweep). 
        * But, as I mentioned, this is only one of many different kinds of Tracing GCs, and is a very simple one with many disadvantages. The two major disadvantages are that scanning the entire memory is expensive and leaving the live objects where they are and only collecting the dead objects in between leads to memory fragmentation.
            Another very simple but much faster Tracing GC is Henry Baker's Semi-Space GC. The Semi-Space GC "wastes" half of the available memory, but gains a lot of performance for it. The way the Semi-Space GC works is that it divides the available memory into two halves, let's call them A and B. Only one of the two halves is active at any one time, meaning new objects only get allocated in one of the two halves.
            We start out with half A: The GC "traces" the "live" objects just as described above, but instead of "marking" them, it copies them to half B. That way, once the "tracing" phase is done, there is no need to scan the entire memory anymore. We know that all live objects are in half B, so we can simply forget about half A completely. From now on, all new objects are allocated in half B. Until the next garbage collection cycle, when all live objects are copied to half A, and we forget about half B.

    TODO :-
        * Scan the stack, data (initialized and bss) and heap to find pointers pointing to a allocated object.
            * Similar to finding any reference. 
        * Build as static library. 
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

long long i, j;
int *ptr;
double k = 2.5;

void test_2() {
    static long long ptr;
    struct node_t {
        unsigned long long ptr;
        node_t *next;
    };
    field_info_t infos1[] = {
        FIELD_INFO(node_t, next, data_type_t::OBJ_PTR, node_t)
    };
    conservative_leak_detector ld(new console_printer());
    ld.register_addr_of_global_variables(&ptr);
    ld.register_addr_of_global_variables(&i);
    ld.register_addr_of_global_variables(&j);
    ld.register_addr_of_global_variables(&k);
    ld.register_struct("node_t", sizeof(node_t), infos1, 
                    sizeof(infos1) / sizeof(field_info_t));
    ld.print_structure_db();

    node_t *node1 = (node_t*) ld.xcalloc("node_t", 1);
    node_t *node2 = (node_t*) ld.xcalloc("node_t", 1);

    // node2->ptr = (unsigned long long) node1;
    // node2->next = node1;

    ld.print_object_db();
    ld.run();
    ld.report_leaked_objects();
}

int main() {
    test_2();
}

/*
Build scripts :-
g++ -g -c src/ld_opps.cpp -o bin/ld_opps.o
g++ -g -c main1.cpp -o bin/main1.o
g++ -g -o bin/exe_opps bin/ld_opps.o bin/main1.o
*/