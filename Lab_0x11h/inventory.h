#ifndef LAB_0X11H_INVENTORY_H
#define LAB_0X11H_INVENTORY_H

#include <stdint.h>
#include <stdbool.h>
#include "item.h"

#define TABLE_SIZE 16

typedef enum {
    SORT_BY_VALUE,
    SORT_BY_RARITY,
    SORT_BY_WEIGHT,
    SORT_BY_QUANTITY,
    SORT_BY_INSERTION_ORDER,
} SortCriterion;

typedef struct InventoryNode {
    char name[MAX_ITEM_NAME];
    Item item;
    int quantity;
    int insertion_order;
    struct InventoryNode* next;
    struct InventoryNode* prev;
} InventoryNode;

typedef struct {
    char name[MAX_ITEM_NAME];
    InventoryNode* node;
    bool is_occupied;
} HashEntry;

// Main inventory structure containing both hash table and linked list
typedef struct {
    HashEntry entries[TABLE_SIZE];    // Hash table for O(1) lookups
    InventoryNode* head;             // Head of sorted linked list
    InventoryNode* tail;             // Tail of sorted linked list
    int size;                        // Number of unique items
    SortCriterion current_sort;      // Current sort criterion
} InventoryDatabase;

// Core inventory functions
uint32_t jenkins_hash(const char* item_name);
void init_inventory_database(InventoryDatabase* db);
bool add_item_to_inventory(InventoryDatabase* db, const Item* item, int quantity);
bool remove_item_from_inventory(InventoryDatabase* db, const char* name, int quantity);
InventoryNode* find_item(const InventoryDatabase* db, const char* name);

// Sort-related function declarations
typedef int (*CompareFunction)(const InventoryNode*, const InventoryNode*);
void sort_inventory(InventoryDatabase* db, CompareFunction compare_func);

// Comparison function declarations
int compare_by_value(const InventoryNode* a, const InventoryNode* b);
int compare_by_rarity(const InventoryNode* a, const InventoryNode* b);
int compare_by_weight(const InventoryNode* a, const InventoryNode* b);
int compare_by_quantity(const InventoryNode* a, const InventoryNode* b);
int compare_by_insertion_order(const InventoryNode* a, const InventoryNode* b);

// Sorting algorithm declarations
void bubble_sort_nodes(InventoryDatabase* db, CompareFunction compare_func);
void quick_sort_nodes(InventoryDatabase* db, int low, int high, CompareFunction compare_func);
void merge_sort_nodes(InventoryDatabase* db, InventoryNode** headRef, CompareFunction compare_func);
void heap_sort_nodes(InventoryDatabase* db, InventoryNode** headRef,CompareFunction compare_func);


// Display functions
void print_inventory(const InventoryDatabase* db);
void print_item_details(const Item* item, int quantity);
const char* get_rarity_string(enum Rarity rarity);

#endif //LAB_0X11H_INVENTORY_H