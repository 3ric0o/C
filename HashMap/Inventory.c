#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define TABLE_SIZE 64  // Small size for demonstration
#define MAX_ITEM_NAME 32

// Simple game item structure
typedef struct {
    char name[MAX_ITEM_NAME];
    int damage;
    int durability;
} GameItem;

// Hash table entry
typedef struct {
    char name[MAX_ITEM_NAME];  // Key
    GameItem item;             // Value
    bool is_occupied;          // Flag to check if slot is used
} HashEntry;

// The actual hash table
typedef struct {
    HashEntry entries[TABLE_SIZE];
} ItemDatabase;

// Our Jenkins hash function (simplified for game items)
uint32_t jenkins_hash(const char* item_name) {
    uint32_t hash = 0;

    while (*item_name) {
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

// Initialize the item database
void init_item_database(ItemDatabase* db) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        db->entries[i].is_occupied = false;
    }
}

// Add an item to the database
bool add_item(ItemDatabase* db, const char* name, int damage, int durability) {
    uint32_t hash = jenkins_hash(name);
    int index = hash % TABLE_SIZE;

    // Simple linear probing for collision resolution
    int original_index = index;
    while (db->entries[index].is_occupied) {
        index = (index + 1) % TABLE_SIZE;
        if (index == original_index) return false; // Table is full
    }

    // Add the item
    strncpy(db->entries[index].name, name, MAX_ITEM_NAME - 1);
    db->entries[index].item.damage = damage;
    db->entries[index].item.durability = durability;
    strncpy(db->entries[index].item.name, name, MAX_ITEM_NAME - 1);
    db->entries[index].is_occupied = true;

    return true;
}

// Find an item in the database
GameItem* find_item(ItemDatabase* db, const char* name) {
    uint32_t hash = jenkins_hash(name);
    int index = hash % TABLE_SIZE;

    // Linear probe until we find the item or an empty slot
    int original_index = index;
    while (db->entries[index].is_occupied) {
        if (strcmp(db->entries[index].name, name) == 0) {
            return &db->entries[index].item;
        }
        index = (index + 1) % TABLE_SIZE;
        if (index == original_index) break; // We've searched the entire table
    }

    return NULL;
}

int main() {
    ItemDatabase game_items;
    init_item_database(&game_items);

    // Add some game items
    add_item(&game_items, "Wooden Sword", 5, 100);
    add_item(&game_items, "Iron Sword", 10, 200);
    add_item(&game_items, "Magic Staff", 15, 150);
    add_item(&game_items, "Legendary Blade", 50, 500);

    // Test finding items
    const char* items_to_find[] = {
            "Wooden Sword",
            "Magic Staff",
            "Not Real Item"  // This one doesn't exist
    };

    // Try to find and print items
    for (int i = 0; i < 3; i++) {
        GameItem* found_item = find_item(&game_items, items_to_find[i]);

        if (found_item) {
            printf("Found item: %s\n", found_item->name);
            printf("  Damage: %d\n", found_item->damage);
            printf("  Durability: %d\n\n", found_item->durability);
        } else {
            printf("Could not find item: %s\n\n", items_to_find[i]);
        }
    }

    return 0;
}