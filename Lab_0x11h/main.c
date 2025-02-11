#include <stdio.h>
#include "inventory.h"
#include "item.h"
#include "raylib.h"

enum Item_Name {
    SWORD,
    SHIELD,
    BOW,
    AXE,
    STAFF,
    DAGGER,
    MACE,
    GREATAXE,
    CROSSBOW,
    CLOAK
};

int main()
{

    InventoryDatabase inventory;
    init_inventory_database(&inventory);
    

    // ================= TEST CODE =================

    add_item_to_inventory(&inventory, &items[SWORD], 2);
    add_item_to_inventory(&inventory, &items[SHIELD], 1);
    add_item_to_inventory(&inventory, &items[BOW], 3);
    add_item_to_inventory(&inventory, &items[STAFF], 1);
    add_item_to_inventory(&inventory, &items[DAGGER], 5);
    printf("\nInitial Inventory:\n");
    print_inventory(&inventory);

    // Modify inventory
    remove_item_from_inventory(&inventory, "Sword", 1);
    remove_item_from_inventory(&inventory, "Bow", 3);
    add_item_to_inventory(&inventory, &items[CLOAK], 2);
    printf("\nUpdated Inventory:\n");
    print_inventory(&inventory);

    // Sort inventory

    //QuickSort
    sort_inventory(&inventory, compare_by_value);
    printf("\nSorted by value using QuickSort:\n");
    print_inventory(&inventory);

    //BubbleSort
    sort_inventory(&inventory, compare_by_insertion_order);
    printf("\nSorted by insertion order using BubbleSort:\n");
    print_inventory(&inventory);

    sort_inventory(&inventory, compare_by_rarity);
    printf("\nSorted by rarity using MergeSort:\n");
    print_inventory(&inventory);

    sort_inventory(&inventory, compare_by_weight);
    printf("\nSorted by weight using HeapSort:\n");
    print_inventory(&inventory);

    sort_inventory(&inventory, compare_by_quantity);
    printf("\nSorted by quantity using QuickSort:\n");
    print_inventory(&inventory);

    return 0;
}