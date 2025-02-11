#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "inventory.h"

uint32_t jenkins_hash(const char* item_name)
{
    uint32_t hash = 0;

    while (*item_name)
    {
        hash += *item_name;
        hash += (hash << 10);
        hash ^= (hash >> 6);
        item_name++;
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}

void init_inventory_database(InventoryDatabase* db)
{
    if (!db) return;

    // Initialize hash table
    for (int i = 0; i < TABLE_SIZE; i++) {
        db->entries[i].is_occupied = false;
        db->entries[i].node = NULL;
    }

    // Initialize linked list
    db->head = NULL;
    db->tail = NULL;
    db->size = 0;
    db->current_sort = SORT_BY_INSERTION_ORDER;
}

InventoryNode* find_item(const InventoryDatabase* db, const char* name)
{
    if (!db || !name) return NULL;

    uint32_t hash = jenkins_hash(name);
    int index = hash % TABLE_SIZE;
    int original_index = index;

    while (db->entries[index].is_occupied) {
        if (strcmp(db->entries[index].name, name) == 0) {
            return db->entries[index].node;
        }
        index = (index + 1) % TABLE_SIZE;
        if (index == original_index) break;
    }

    return NULL;
}

bool add_item_to_inventory(InventoryDatabase* db, const Item* item, int quantity) {
    if (!db || !item || quantity <= 0) return false;

    static int insertion_counter = 0;  // Add this counter

    // Try to find existing item using hash table
    InventoryNode* existing_node = find_item(db, item->name);
    if (existing_node) {
        existing_node->quantity += quantity;
        return true;
    }

    // Create new node for linked list
    InventoryNode* new_node = (InventoryNode*)malloc(sizeof(InventoryNode));
    if (!new_node) return false;

    strncpy(new_node->name, item->name, MAX_ITEM_NAME - 1);
    new_node->name[MAX_ITEM_NAME - 1] = '\0';
    new_node->item = *item;
    new_node->quantity = quantity;
    new_node->insertion_order = insertion_counter++;
    new_node->next = NULL;
    new_node->prev = NULL;

    // Add to linked list
    if (db->head == NULL) {
        db->head = new_node;
        db->tail = new_node;
    } else {
        new_node->prev = db->tail;
        db->tail->next = new_node;
        db->tail = new_node;
    }

    // Add to hash table
    uint32_t hash = jenkins_hash(item->name);
    int index = hash % TABLE_SIZE;
    int original_index = index;

    while (db->entries[index].is_occupied) {
        index = (index + 1) % TABLE_SIZE;
        if (index == original_index) {
            // Hash table is full
            free(new_node);
            return false;
        }
    }

    strncpy(db->entries[index].name, item->name, MAX_ITEM_NAME - 1);
    db->entries[index].name[MAX_ITEM_NAME - 1] = '\0';
    db->entries[index].node = new_node;
    db->entries[index].is_occupied = true;

    db->size++;
    return true;
}

bool remove_item_from_inventory(InventoryDatabase* db, const char* name, int quantity)
{
    if (!db || !name || quantity <= 0) return false;

    // Find item using hash table
    uint32_t hash = jenkins_hash(name);
    int index = hash % TABLE_SIZE;
    int original_index = index;

    while (db->entries[index].is_occupied) {
        if (strcmp(db->entries[index].name, name) == 0) {
            InventoryNode* node = db->entries[index].node;

            if (node->quantity >= quantity) {
                node->quantity -= quantity;

                // Remove node if quantity becomes 0
                if (node->quantity == 0) {
                    // Update linked list
                    if (node->prev) {
                        node->prev->next = node->next;
                    } else {
                        db->head = node->next;
                    }

                    if (node->next) {
                        node->next->prev = node->prev;
                    } else {
                        db->tail = node->prev;
                    }

                    // Update hash table
                    db->entries[index].is_occupied = false;
                    db->entries[index].node = NULL;

                    free(node);
                    db->size--;
                }
                return true;
            }
            return false;
        }
        index = (index + 1) % TABLE_SIZE;
        if (index == original_index) break;
    }
    return false;
}

int compare_by_value(const InventoryNode* a, const InventoryNode* b) {
    return b->item.value - a->item.value;
}

int compare_by_rarity(const InventoryNode* a, const InventoryNode* b) {
    // Sort in descending order (LEGENDARY first)
    if (b->item.rarity != a->item.rarity) {
        return b->item.rarity - a->item.rarity;
    }
    // If rarities are equal, maintain stable sort using insertion order
    return a->insertion_order - b->insertion_order;
}

int compare_by_weight(const InventoryNode* a, const InventoryNode* b) {
    return b->item.weight - a->item.weight;
}

int compare_by_quantity(const InventoryNode* a, const InventoryNode* b) {
    return b->quantity - a->quantity;
}

int compare_by_insertion_order(const InventoryNode* a, const InventoryNode* b) {
    return a->insertion_order - b->insertion_order;
}

void sort_inventory(InventoryDatabase* db, CompareFunction compare_func) {
    if (!db || !compare_func || db->size <= 1) return;

    if (compare_func == compare_by_insertion_order) {
        bubble_sort_nodes(db, compare_func);
        db->current_sort = SORT_BY_INSERTION_ORDER;
    }
    else if (compare_func == compare_by_value) {
        // Use selection sort for value-based sorting
        quick_sort_nodes(db, 0, db->size - 1, compare_func);
        db->current_sort = SORT_BY_VALUE;
    }
    else if (compare_func == compare_by_rarity) {
        // Use insertion sort for rarity-based sorting
        merge_sort_nodes(db, &(db->head), compare_func);
        db->current_sort = SORT_BY_RARITY;
    }
    else if (compare_func == compare_by_weight) {
        // Use selection sort for weight-based sorting
        heap_sort_nodes(db, &(db->head), compare_func);
        db->current_sort = SORT_BY_WEIGHT;
    }
    else if (compare_func == compare_by_quantity) {
        // Use bubble sort for quantity-based sorting (stable sort)
        quick_sort_nodes(db, 0, db->size - 1, compare_func);
        db->current_sort = SORT_BY_QUANTITY;
    }
}
void swap_node_data(InventoryNode* a, InventoryNode* b, InventoryDatabase* db) {
    // Swap node data
    Item temp_item = a->item;
    int temp_quantity = a->quantity;
    int temp_order = a->insertion_order;
    char temp_name[MAX_ITEM_NAME];
    strncpy(temp_name, a->name, MAX_ITEM_NAME - 1);

    a->item = b->item;
    a->quantity = b->quantity;
    a->insertion_order = b->insertion_order;
    strncpy(a->name, b->name, MAX_ITEM_NAME - 1);

    b->item = temp_item;
    b->quantity = temp_quantity;
    b->insertion_order = temp_order;
    strncpy(b->name, temp_name, MAX_ITEM_NAME - 1);

    // Update hash table entries
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (db->entries[i].is_occupied) {
            if (db->entries[i].node == a) {
                strncpy(db->entries[i].name, a->name, MAX_ITEM_NAME - 1);
            }
            else if (db->entries[i].node == b) {
                strncpy(db->entries[i].name, b->name, MAX_ITEM_NAME - 1);
            }
        }
    }
}

// Get node at position
InventoryNode* get_node_at(InventoryNode* head, int pos) {
    InventoryNode* current = head;
    for (int i = 0; i < pos && current != NULL; i++) {
        current = current->next;
    }
    return current;
}

// Partition function for quicksort
int partition(InventoryDatabase* db, int low, int high, CompareFunction compare_func) {
    InventoryNode* pivot = get_node_at(db->head, high);
    int i = low - 1;

    for (int j = low; j < high; j++) {
        InventoryNode* current = get_node_at(db->head, j);
        if (compare_func(current, pivot) <= 0) {
            i++;
            InventoryNode* node_i = get_node_at(db->head, i);
            swap_node_data(node_i, current, db);
        }
    }

    InventoryNode* node_i_plus_1 = get_node_at(db->head, i + 1);
    InventoryNode* node_high = get_node_at(db->head, high);
    swap_node_data(node_i_plus_1, node_high, db);
    return i + 1;
}

// Quicksort function
void quick_sort_nodes(InventoryDatabase* db, int low, int high, CompareFunction compare_func) {
    if (low < high) {
        int pi = partition(db, low, high, compare_func);
        quick_sort_nodes(db, low, pi - 1, compare_func);
        quick_sort_nodes(db, pi + 1, high, compare_func);
    }
}
// Bubble sort function
void bubble_sort_nodes(InventoryDatabase* db, CompareFunction compare_func) {
    bool swapped;
    InventoryNode *current;
    InventoryNode *last = NULL;

    do {
        swapped = false;
        current = db->head;

        while (current->next != last) {
            if (compare_func(current, current->next) > 0) {
                // Swap node data
                Item temp_item = current->item;
                int temp_quantity = current->quantity;
                int temp_order = current->insertion_order;
                char temp_name[MAX_ITEM_NAME];
                strncpy(temp_name, current->name, MAX_ITEM_NAME - 1);

                current->item = current->next->item;
                current->quantity = current->next->quantity;
                current->insertion_order = current->next->insertion_order;
                strncpy(current->name, current->next->name, MAX_ITEM_NAME - 1);

                current->next->item = temp_item;
                current->next->quantity = temp_quantity;
                current->next->insertion_order = temp_order;
                strncpy(current->next->name, temp_name, MAX_ITEM_NAME - 1);

                // Update hash table entries
                for (int i = 0; i < TABLE_SIZE; i++) {
                    if (db->entries[i].is_occupied) {
                        if (db->entries[i].node == current) {
                            strncpy(db->entries[i].name, current->name, MAX_ITEM_NAME - 1);
                        }
                        else if (db->entries[i].node == current->next) {
                            strncpy(db->entries[i].name, current->next->name, MAX_ITEM_NAME - 1);
                        }
                    }
                }

                swapped = true;
            }
            current = current->next;
        }
        last = current;
    } while (swapped);
}
// Merge sort function

/* Find Middle using slow and fast pointers
 * It takes the head of the linked list as input and store it as slow and fast pointers.
 * The fast pointer moves two steps at a time and the slow pointer moves one step at a time.
 * When the fast pointer reaches the end of the linked list, the slow pointer will be at the middle of the linked list.
 * Return the slow pointer.
 * This way, we can find the middle of the linked list in O(n) time complexity.
 */
void findMiddle(InventoryNode* head, InventoryNode** middle) {
    InventoryNode* slow = head;
    InventoryNode* fast = head->next;

    while (fast != NULL) {
        fast = fast->next;
        if (fast != NULL) {
            slow = slow->next;
            fast = fast->next;
        }
    }

    *middle = slow;
}

void merge(InventoryNode** headRef, InventoryNode* left, InventoryNode* right,InventoryDatabase* db, CompareFunction compare_func) {
    InventoryNode* result = NULL;

    // Base cases
    if (left == NULL) {
        *headRef = right;
        return;
    }
    if (right == NULL) {
        *headRef = left;
        return;
    }

    // Compare nodes using the provided comparison function
    if (compare_func(left, right) <= 0) {
        result = left;
        merge(&(result->next), left->next, right, db, compare_func);
    } else {
        result = right;
        merge(&(result->next), left, right->next, db, compare_func);
    }

    // Update prev pointer for the next node
    if (result->next) {
        result->next->prev = result;
    }

    *headRef = result;
}
void merge_sort_nodes(InventoryDatabase* db, InventoryNode** headRef, CompareFunction compare_func) {
    InventoryNode* head = *headRef;
    InventoryNode* left;
    InventoryNode* right;

    // Base case: if list is empty or has only one element
    if (head == NULL || head->next == NULL) {
        return;
    }

    InventoryNode* middle;
    findMiddle(head, &middle);

    // Store the node after middle as right start
    right = middle->next;

    // Break the list into two halves
    middle->next = NULL;
    if (right) {
        right->prev = NULL;
    }

    left = head;

    // Recursively sort both halves
    merge_sort_nodes(db, &left, compare_func);
    merge_sort_nodes(db, &right, compare_func);

    // Merge the sorted halves
    merge(headRef, left, right, db, compare_func);

    // Update the prev pointers and find the new tail
    InventoryNode* current = *headRef;
    InventoryNode* prev = NULL;

    while (current != NULL) {
        current->prev = prev;
        prev = current;
        if (current->next == NULL) {
            db->tail = current; // Update tail
        }
        current = current->next;
    }
}

// Heap sort function
int count_nodes(InventoryNode* head) {
    int count = 0;
    InventoryNode* current = head;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}
void heapify(InventoryDatabase* db, InventoryNode* head, int n, int i,CompareFunction compare_func) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    InventoryNode* largest_node = get_node_at(head, largest);
    InventoryNode* left_node = (left < n) ? get_node_at(head, left) : NULL;
    InventoryNode* right_node = (right < n) ? get_node_at(head, right) : NULL;

    // Compare with left child
    if (left < n && left_node && compare_func(left_node, largest_node) > 0) {
        largest = left;
        largest_node = left_node;
    }

    // Compare with right child
    if (right < n && right_node && compare_func(right_node, largest_node) > 0) {
        largest = right;
        largest_node = right_node;
    }

    // If largest is not root
    if (largest != i) {
        InventoryNode* i_node = get_node_at(head, i);
        swap_node_data(i_node, largest_node, db);
        heapify(db, head, n, largest, compare_func);
    }
}
void heap_sort_nodes(InventoryDatabase* db, InventoryNode** headRef,CompareFunction compare_func) {
    if (!db || !headRef || !(*headRef)) return;

    int n = count_nodes(*headRef);

    // Build max heap
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(db, *headRef, n, i, compare_func);
    }

    // Extract elements from heap one by one
    for (int i = n - 1; i >= 0; i--) {
        // Move current root to end
        InventoryNode* last_node = get_node_at(*headRef, i);
        InventoryNode* root_node = *headRef;

        swap_node_data(root_node, last_node, db);

        // Call heapify on the reduced heap
        heapify(db, *headRef, i, 0, compare_func);
    }

    // Update prev pointers and tail
    InventoryNode* current = *headRef;
    InventoryNode* prev = NULL;

    while (current != NULL) {
        current->prev = prev;
        prev = current;
        if (current->next == NULL) {
            db->tail = current;
        }
        current = current->next;
    }
}

