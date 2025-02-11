#include <stdio.h>
#include "inventory.h"

const char* get_rarity_string(enum Rarity rarity)
{
    switch (rarity)
    {
        case COMMON:    return "Common";
        case UNCOMMON:  return "Uncommon";
        case RARE:      return "Rare";
        case EPIC:      return "Epic";
        case LEGENDARY: return "Legendary";
        default:        return "Unknown";
    }
}

void print_item_details(const Item* item, int quantity)
{
    printf("| %-20s | %-9s | %4d | %7.1f | %3d |\n",
           item->name,
           get_rarity_string(item->rarity),
           item->value,
           item->weight,
           quantity);
}

void print_inventory(const InventoryDatabase* db)
{
    if (!db) return;

    printf("\n+----------------------+-----------+------+---------+-----+\n");
    printf("| Item Name            | Rarity    | Value| Weight  | Qty |\n");
    printf("+----------------------+-----------+------+---------+-----+\n");

    int total_items = 0;
    float total_weight = 0.0f;
    int total_value = 0;

    InventoryNode* current = db->head;
    while (current != NULL) {
        if (current->quantity > 0) {
            print_item_details(&current->item, current->quantity);

            total_items += current->quantity;
            total_weight += current->item.weight * current->quantity;
            total_value += current->item.value * current->quantity;
        }
        current = current->next;
    }

    printf("+----------------------+-----------+------+---------+-----+------+\n");
    printf("| Total Items: %-5d   Total Weight: %-6.1f   Total Value: %-5d |\n",
           total_items, total_weight, total_value);
    printf("+----------------------------------------------------------------+\n");
}