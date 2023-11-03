#include <stdio.h>
#include <assert.h>
#include <malloc.h>

typedef struct node_t {
    int data;
    double more_data;
    struct node_t *next;
} node_t;

int main() {
    node_t *node = (node_t*) calloc(1, sizeof(node_t));
    printf("Address of node: %p\n", node);
    printf("Address of node: %p\n", &node->next);
}

/*

Address of d:   0x560560fbc010
Address of i:   0x560560fbc04c
Address of arr: 0x560560fbc060
etext:          0x560560dbb7fd
end:            0x560560fbc080


*/