#ifndef __LD_OPPS__
#define __LD_OPPS__ true

#include <iostream>
#include <string.h>
#include <assert.h>
#include <set>

#define MAX_STRUCTURE_NAME_SIZE 128
#define MAX_FIELD_NAME_SIZE     128

#define SIZEOF(struct_name, field_name) \
    sizeof(struct_name::field_name)

#define OFFSETOF(struct_name, field_name) \
    (unsigned long int)&((struct_name*)0)->field_name

#define TOSTRING(not_string) \
    #not_string

#define FIELD_INFO(struct_name, field_name, dtype, nested_sname) \
    { #field_name, dtype, SIZEOF(struct_name, field_name), \
    OFFSETOF(struct_name, field_name), #nested_sname }

enum data_type_t {
    UINT8, UINT32, INT32, CHAR, OBJ_PTR, FLOAT, DOUBLE, OBJ_STRUCT, VOID_PTR
};

struct field_info_t {
    char fname[MAX_FIELD_NAME_SIZE];
    data_type_t dtype;
    unsigned int size;
    unsigned int offset;
    char nested_str_name[MAX_STRUCTURE_NAME_SIZE];
};

struct struct_db_rec_t {
    struct_db_rec_t *next;
    char struct_name[MAX_STRUCTURE_NAME_SIZE];
    unsigned int ds_size;
    unsigned int n_fields;
    field_info_t *fields;
    struct_db_rec_t(char *pname, unsigned int psize, unsigned int pnfields,
                    field_info_t *pfields) {
        next = NULL;
        strncpy(struct_name, pname, MAX_STRUCTURE_NAME_SIZE);
        ds_size = psize;
        n_fields = pnfields;
        fields = pfields;
    }
    ~struct_db_rec_t() { std::cout << "deleted" << std::endl; }
};

struct struct_db_t {
    struct_db_rec_t *head;
    unsigned int count;
    ~struct_db_t() {
        // Delete all struct_db_rec_t nodes
    }
};

struct object_db_rec_t {
    object_db_rec_t *next;
    void *ptr;
    unsigned int units;
    struct_db_rec_t *struct_rec;
    bool is_visited;
    bool is_root;
};

struct object_db_t {
    struct_db_t *struct_db;
    object_db_rec_t *head;
    unsigned int count;
};

class printer {
public:
    virtual void print_structure_record(struct_db_rec_t *struct_rec) = 0;
    virtual void print_structure_db(struct_db_t *struct_db) = 0;
    virtual void print_object_rec(object_db_rec_t *obj_rec, int i) = 0;
    virtual void print_object_db(object_db_t *object_db) = 0;
    virtual void dump_object_rec_detail(object_db_rec_t *head) = 0;
};

class console_printer : public printer {
public:
    void print_structure_record(struct_db_rec_t *struct_rec);
    void print_structure_db(struct_db_t *struct_db);
    void print_object_rec(object_db_rec_t *obj_rec, int i);
    void print_object_db(object_db_t *object_db);
    void dump_object_rec_detail(object_db_rec_t *head);
};

class memory_leak_detector {
protected:
    struct_db_t *struct_db;
    object_db_t *object_db;
    printer *_printer;
    virtual int add_structure_to_struct_db(struct_db_rec_t *struct_rec);
    void init_primitive_data_types() {
        register_struct("int", sizeof(int), NULL, 0);
        register_struct("float", sizeof(float), NULL, 0);
        register_struct("double", sizeof(double), NULL, 0);
        register_struct("char", sizeof(char), NULL, 0);
    }
    object_db_rec_t* get_next_root_object(object_db_rec_t *starting);
public:
    memory_leak_detector(printer *p_printer);
    void register_struct(char *struct_name, unsigned int struct_size,
                         field_info_t *fields_arr, unsigned int fields_arr_size);
    void print_structure_db() {
        _printer->print_structure_db(struct_db);
    }
    void print_object_db() {
        _printer->print_object_db(object_db);
    }
    void* xcalloc(char *struct_name, int units);

    void add_object_to_object_db(void *ptr, int units,
                     struct_db_rec_t *struct_rec, bool is_root);
    
    object_db_rec_t *object_db_look_up(void *ptr);

    void set_object_as_global_root(void *obj_ptr);

    void xfree(void *ptr);

    void delete_object_record_from_object_db(object_db_rec_t *object_rec);

    struct_db_rec_t *struct_db_look_up(char *struct_name){
        struct_db_rec_t *head = struct_db->head;
        if(!head) return NULL;    
        for(; head; head = head->next){
            if(strncmp(head->struct_name, struct_name, MAX_STRUCTURE_NAME_SIZE) ==0)
                return head;
        }
        return NULL;
    }
    virtual void run() = 0;
    virtual void report_leaked_objects() = 0;
};

class memory_leak_algorithm_graph : public memory_leak_detector {
private:
    void init_algorithm();
    void explore_objects(object_db_rec_t *parent_obj_rec);
public:
    memory_leak_algorithm_graph(printer *p_printer) : 
    memory_leak_detector(p_printer) {}
    void run();
    void report_leaked_objects();
};

class conservative_leak_detector : public memory_leak_algorithm_graph {
private:
    std::set<unsigned long long> global_addresses;
    void scan_stack();
public:
    conservative_leak_detector(printer *p_printer) :
        memory_leak_algorithm_graph(p_printer) {}
    void register_addr_of_global_variables(void *address);
    void run();
    void scan_memory();
};

#endif