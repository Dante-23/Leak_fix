#include "../includes/ld_opps.h"
#include <math.h>

const char *DATA_TYPE[] = {"UINT8", "UINT32", "INT32",
                     "CHAR", "OBJ_PTR", "VOID_PTR", "FLOAT",
                     "DOUBLE", "OBJ_STRUCT"};

void console_printer::print_structure_record(struct_db_rec_t *struct_rec) {
    if(!struct_rec) return;
    int j = 0;
    field_info_t *field = NULL;
    printf("|------------------------------------------------------|\n");
    printf("| %-20s | size = %-8d | #flds = %-3d |\n", struct_rec->struct_name, struct_rec->ds_size, struct_rec->n_fields);
    printf("|------------------------------------------------------|------------------------------------------------------------------------------------------|\n");
    for(j = 0; j < struct_rec->n_fields; j++){
        field = &struct_rec->fields[j];
        printf("  %-20s |", "");
        printf("%-3d %-20s | dtype = %-15s | size = %-5d | offset = %-6d|  nstructname = %-20s  |\n",
                j, field->fname, DATA_TYPE[field->dtype], field->size, field->offset, field->nested_str_name);
        printf("  %-20s |", "");
        printf("--------------------------------------------------------------------------------------------------------------------------|\n");
    }
}

void console_printer::print_structure_db(struct_db_t *struct_db) {
    if(!struct_db) return;
    printf("Printing STRUCURE DATABASE\n");
    int i = 0;
    struct_db_rec_t *struct_rec = NULL;
    struct_rec = struct_db->head;
    printf("No of Structures Registered = %d\n", struct_db->count);
    while(struct_rec){
        printf("structure No : %d (%p)\n", i++, struct_rec);
        print_structure_record(struct_rec);
        struct_rec = struct_rec->next;
    }
}

void console_printer::print_object_rec(object_db_rec_t *obj_rec, int i){
    if(!obj_rec) return;
    printf("-----------------------------------------------------------------------------------------------------|\n");
    printf("%-3d ptr = %-10p | next = %-10p | units = %-4d | struct_name = %-10s | is_root = %s |\n", 
        i, obj_rec->ptr, obj_rec->next, obj_rec->units, obj_rec->struct_rec->struct_name, obj_rec->is_root ? "TRUE" : "FALSE"); 
    printf("-----------------------------------------------------------------------------------------------------|\n");
}

void console_printer::print_object_db(object_db_t *object_db){
    object_db_rec_t *head = object_db->head;
    unsigned int i = 0;
    printf("Printing OBJECT DATABASE\n");
    for(; head; head = head->next){
        print_object_rec(head, i++);
    }
}

void console_printer::dump_object_rec_detail(object_db_rec_t *obj_rec) {
    int n_fields = obj_rec->struct_rec->n_fields;
    field_info_t *field = NULL;
    int units = obj_rec->units, obj_index = 0, field_index = 0;
    for(; obj_index < units; obj_index++){
        char *current_object_ptr = (char *)(obj_rec->ptr) + \
                        (obj_index * obj_rec->struct_rec->ds_size);
        for(field_index = 0; field_index < n_fields; field_index++){            
            field = &obj_rec->struct_rec->fields[field_index];
            switch(field->dtype){
                case UINT8:
                case INT32:
                case UINT32:
                    printf("%s[%d]->%s = %d\n", obj_rec->struct_rec->struct_name, obj_index, field->fname, *(int *)(current_object_ptr + field->offset));
                    break;
                case CHAR:
                    printf("%s[%d]->%s = %s\n", obj_rec->struct_rec->struct_name, obj_index, field->fname, (char *)(current_object_ptr + field->offset));
                    break;
                case FLOAT:
                    printf("%s[%d]->%s = %f\n", obj_rec->struct_rec->struct_name, obj_index, field->fname, *(float *)(current_object_ptr + field->offset));
                    break;
                case DOUBLE:
                    printf("%s[%d]->%s = %f\n", obj_rec->struct_rec->struct_name, obj_index, field->fname, *(double *)(current_object_ptr + field->offset));
                    break;
                case OBJ_PTR:
                    printf("%s[%d]->%s = %p\n", obj_rec->struct_rec->struct_name, obj_index, field->fname,  (void *)*(int *)(current_object_ptr + field->offset));
                    break;
                case OBJ_STRUCT:
                    /*Later*/
                    break;
                default:
                    break;
            }
        }
    }
}

memory_leak_detector::memory_leak_detector(printer *p_printer) {
    _printer = p_printer;
    struct_db = new struct_db_t();
    object_db = new object_db_t();
    init_primitive_data_types();
}

void memory_leak_detector::register_struct(char *struct_name, unsigned int struct_size,
                         field_info_t *fields_arr, unsigned int fields_arr_size) {
    struct_db_rec_t *struct_rec = new struct_db_rec_t(struct_name, struct_size,
                                    fields_arr_size, fields_arr);
    if (!add_structure_to_struct_db(struct_rec)) {
        assert(0);
    }
}

int memory_leak_detector::add_structure_to_struct_db(struct_db_rec_t* struct_rec) {
    struct_db_rec_t *head = struct_db->head;
    if(!head){
        struct_db->head = struct_rec;
        struct_rec->next = NULL;
        struct_db->count++;
        return 1;
    }
    struct_rec->next = head;
    struct_db->head = struct_rec;
    struct_db->count++;
    return 1;
}

void* memory_leak_detector::xcalloc(char *struct_name, int units) {
    struct_db_rec_t *struct_rec = struct_db_look_up(struct_name);
    assert(struct_rec);
    void *ptr = calloc(units, struct_rec->ds_size);
    add_object_to_object_db(ptr, units, struct_rec, false);
    return ptr;
}

void memory_leak_detector::add_object_to_object_db(void *ptr, int units,
                     struct_db_rec_t *struct_rec, bool is_root){
    object_db_rec_t *obj_rec = object_db_look_up(ptr);
    /*Dont add same object twice*/
    assert(!obj_rec);
    obj_rec = (object_db_rec_t*)calloc(1, sizeof(object_db_rec_t));

    obj_rec->next = NULL;
    obj_rec->ptr = ptr;
    obj_rec->units = units;
    obj_rec->struct_rec = struct_rec;
    obj_rec->is_visited = false;
    obj_rec->is_root = is_root;

    object_db_rec_t *head = object_db->head;
        
    if(!head){
        object_db->head = obj_rec;
        obj_rec->next = NULL;
        object_db->count++;
        return;
    }

    obj_rec->next = head;
    object_db->head = obj_rec;
    object_db->count++;
}

object_db_rec_t* memory_leak_detector::object_db_look_up(void *ptr){
    object_db_rec_t *head = object_db->head;
    if(!head) return NULL;    
    for(; head; head = head->next){
        if(head->ptr == ptr)
            return head;
    }
    return NULL;
}

void memory_leak_detector::set_object_as_global_root(void *obj_ptr) {
    object_db_rec_t *object_rec = object_db_look_up(obj_ptr);
    assert(object_rec);
    object_rec->is_root = true;
}

void memory_leak_detector::xfree(void *ptr){
    if(!ptr) return;
    object_db_rec_t *object_rec = object_db_look_up(ptr);
    assert(object_rec);
    assert(object_rec->ptr);
    free(object_rec->ptr);
    object_rec->ptr = NULL;
    delete_object_record_from_object_db(object_rec);
}

void memory_leak_detector::delete_object_record_from_object_db(object_db_rec_t *object_rec){
    assert(object_rec);
    object_db_rec_t *head = object_db->head;
    if(head == object_rec){
        object_db->head = object_rec->next;
        free(object_rec);
        return;
    }
    object_db_rec_t *prev = head;
    head = head->next;
    while(head){
        if(head != object_rec){
            prev = head;
            head = head->next;
            continue;
        }
        prev->next = head->next;
        head->next = NULL;
        free(head);
        return;
    }
}

object_db_rec_t* memory_leak_detector::get_next_root_object(object_db_rec_t *starting){
    object_db_rec_t *first = starting ? starting->next : object_db->head;
    while(first){
        if(first->is_root)
            return first;
        first = first->next;
    }
    return NULL;
}

void memory_leak_algorithm_graph::explore_objects(object_db_rec_t *parent_obj_rec){
    unsigned int i , n_fields;
    char *parent_obj_ptr = NULL, *child_obj_offset = NULL;
    void *child_object_address = NULL;
    field_info_t *field_info = NULL;

    object_db_rec_t *child_object_rec = NULL;
    struct_db_rec_t *parent_struct_rec = parent_obj_rec->struct_rec;
    assert(parent_obj_rec->is_visited);
    if(parent_struct_rec->n_fields == 0){
        return;
    }

    for( i = 0; i < parent_obj_rec->units; i++){
        parent_obj_ptr = (char *)(parent_obj_rec->ptr) + (i * parent_struct_rec->ds_size);
        for(n_fields = 0; n_fields < parent_struct_rec->n_fields; n_fields++){
            field_info = &parent_struct_rec->fields[n_fields];
            switch(field_info->dtype){
                case UINT8:
                case UINT32:
                case INT32:
                case CHAR:
                case FLOAT:
                case DOUBLE:
                case OBJ_STRUCT:
                    break;
                case VOID_PTR:
                case OBJ_PTR:
                default:
                    ;
                child_obj_offset = parent_obj_ptr + field_info->offset;
                memcpy(&child_object_address, child_obj_offset, sizeof(void *));
                if(!child_object_address) continue;
                child_object_rec = object_db_look_up(child_object_address);
                assert(child_object_rec);
                if(!child_object_rec->is_visited){
                    child_object_rec->is_visited = true;
                    if(field_info->dtype != VOID_PTR) 
                        explore_objects(child_object_rec);
                }
                else{
                    continue;
                }
            }
        }
    }
}

void memory_leak_algorithm_graph::init_algorithm(){
    object_db_rec_t *obj_rec = object_db->head;
    while(obj_rec){
        obj_rec->is_visited = false;
        obj_rec = obj_rec->next;
    }
}

void memory_leak_algorithm_graph::run() {
    printf("Running memory_leak_algorithm_graph::run()\n");
    init_algorithm();
    object_db_rec_t *root_obj = get_next_root_object(NULL);
    while(root_obj) {
        if (root_obj->is_visited) {
            root_obj = get_next_root_object(root_obj);
            continue;
        }
        root_obj->is_visited = true;
        explore_objects(root_obj);
        root_obj = get_next_root_object(root_obj);
    }
}

void memory_leak_algorithm_graph::report_leaked_objects() {
    int i = 0;
    object_db_rec_t *head;
    printf("Dumping Leaked Objects\n");
    for(head = object_db->head; head; head = head->next){
        if(!head->is_visited){
            _printer->print_object_rec(head, i++);
            _printer->dump_object_rec_detail(head);
            printf("\n\n");
        }
    }
}

void conservative_leak_detector::scan_memory() {
    // Scanning data section variables
    printf("Scanning data section start\n");
    for (unsigned long long address: global_addresses) {
        object_db_rec_t *obj = object_db_look_up((void*)(*((unsigned long long*)address)));
        if (obj == NULL) continue;
        obj->is_visited = true;
    }
    printf("Scanning data section end\n");

    // Scanning stack section
    scan_stack();

    // Scanning heap section
    scan_heap();
}

void conservative_leak_detector::scan_stack() {
    printf("Scanning stack section start\n");
    unsigned long long stack_top, stack_bottom;
    static int initted;
    FILE *statfp;
    if (initted)
        return;
    initted = 1;
    statfp = fopen("/proc/self/stat", "r");
    assert(statfp != NULL);
    fscanf(statfp,
           "%*d %*s %*c %*d %*d %*d %*d %*d %*u "
           "%*lu %*lu %*lu %*lu %*lu %*lu %*ld %*ld "
           "%*ld %*ld %*ld %*ld %*llu %*lu %*ld "
           "%*lu %*lu %*lu %lu", &stack_bottom);
    fclose(statfp);
    asm volatile ("mov %%rsp, %0" : "=r" (stack_top));
    
    for (unsigned int *ptr = (unsigned int*)stack_top; 
        ptr <= (unsigned int*)stack_bottom; ptr++) {
        unsigned int value = *ptr;
        object_db_rec_t *obj = object_db_look_up((void*)value);
        if (obj == NULL) continue;
        obj->is_visited = true;
    }
    printf("Scanning stack section end\n");
}

void conservative_leak_detector::run() {
    memory_leak_algorithm_graph::run();
    printf("Running conservative_leak_detector::run()\n");
    scan_memory();
}

void conservative_leak_detector::register_addr_of_global_variables(void *address) {
    global_addresses.insert((unsigned long long)address);
}

void conservative_leak_detector::scan_heap() {
    object_db_rec_t *head = object_db->head;
    while(head != NULL) {
        struct_db_rec_t *struct_rec = head->struct_rec;
        void *start_address = head->ptr;
        unsigned int largest_offset = 0;
        for (unsigned int i = 0; i < struct_rec->n_fields; i++) {
            largest_offset = std::max(largest_offset, struct_rec->fields[i].offset);
        }
        void *end_address = head->ptr + largest_offset;
        for (unsigned int *ptr = (unsigned int*)start_address; 
            ptr <= (unsigned int*)end_address;
            ptr++) {
            unsigned long long *ptr1 = (unsigned long long*) ptr;
            object_db_rec_t *obj = object_db_look_up((void*)(*ptr1));
            printf("Value at address %p is %p\n", ptr1, *ptr1);
            if (obj == NULL) continue;
            obj->is_visited = true;
        }
        head = head->next;
    }
}