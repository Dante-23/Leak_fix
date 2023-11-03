#include <stdio.h>
#include <assert.h>

int i, j, k, l1;
char a, b, c;
char arr[32];
double radius;
int first = 5;
char e = 'h';
unsigned int f = 32;
long long l = 4096;
unsigned int value = 230598;
int d = 1024;

void scan_stack(int suspected_value) {
    printf("Address in scan_stack: %p\n", &suspected_value);
    unsigned long long stack_top, stack_bottom;
    static int initted;

    int suspect1 = 32, suspect2 = 64;

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
        if (value == suspected_value) {
            printf("Found in stack section at address %p\n", ptr);
        }
    }
}

void foo() {
    int value = 230598;
    printf("Address in foo: %p\n", &value);
    scan_stack(value);
}

void scan_data_section(unsigned long* addr_of_first_initialized_var,
                    unsigned long* addr_of_last_uninitialized_var,
                    int suspected_value) {
    for (unsigned int *ptr = (unsigned int*)addr_of_first_initialized_var; 
        ptr <= (unsigned int*)addr_of_last_uninitialized_var; ptr++) {
        printf("ptr: %p\n", ptr);
        unsigned int value = *((unsigned int*)ptr);
        printf("Value: %d\n", value);
        if (value == suspected_value) {
            printf("Found in data section at address: %p\n", ptr);
        }
    }
}

int main() {
    // extern char etext, end;
    // printf("Address of i: %p\n", &i);
    // printf("Address of d: %p\n", &d);
    // printf("Address of arr: %p\n", &arr);

    // printf("etext: %p\n", &etext);
    // printf("end: %p\n", &end);

    // for (void *ptr = (void*)&e; ptr <= (void*)&arr; ptr++) {
    //     printf("ptr: %p\n", ptr);
    //     unsigned int value = *((unsigned int*)ptr);
    //     printf("Value: %d\n", value);
    //     if (value == 1024) {
    //         printf("Found\n");
    //     }
    // }
    printf("Address of i: %p\n", &i);
    printf("Address of j: %p\n", &j);
    printf("Address of k: %p\n", &k);
    printf("Address of l1: %p\n", &l1);
    printf("Address of a: %p\n", &a);
    printf("Address of b: %p\n", &b);
    printf("Address of c: %p\n", &c);
    printf("Address of value: %p\n", &value);
    scan_data_section(&first, &radius, value);
}

/*

Address of d:   0x560560fbc010
Address of i:   0x560560fbc04c
Address of arr: 0x560560fbc060
etext:          0x560560dbb7fd
end:            0x560560fbc080


*/