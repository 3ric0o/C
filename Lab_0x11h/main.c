#include <stdio.h>
#include <string.h>
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

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define ICON_SIZE 64
#define INVENTORY_COLS 8
#define INVENTORY_ROWS 4
#define INVENTORY_PADDING 10

typedef struct {
    Texture2D iconTexture;
} ItemIcon;

ItemIcon itemIcons[10];  // One for each enum Item_Name
Texture2D blankIcon;

void LoadItemIcons() {
    // Load all item icons
    itemIcons[SWORD].iconTexture = LoadTexture("icons/sword.png");
    itemIcons[SHIELD].iconTexture = LoadTexture("icons/shield.png");
    itemIcons[BOW].iconTexture = LoadTexture("icons/bow.png");
    itemIcons[AXE].iconTexture = LoadTexture("icons/axe.png");
    itemIcons[STAFF].iconTexture = LoadTexture("icons/staff.png");
    itemIcons[DAGGER].iconTexture = LoadTexture("icons/dagger.png");
    itemIcons[MACE].iconTexture = LoadTexture("icons/mace.png");
    itemIcons[GREATAXE].iconTexture = LoadTexture("icons/greataxe.png");
    itemIcons[CROSSBOW].iconTexture = LoadTexture("icons/crossbow.png");
    itemIcons[CLOAK].iconTexture = LoadTexture("icons/cloak.png");

    // Load blank icon for empty slots
    blankIcon = LoadTexture("icons/blank.png");
}

void UnloadItemIcons() {
    // Unload all item icons
    for (int i = 0; i < 10; i++) {
        UnloadTexture(itemIcons[i].iconTexture);
    }
    UnloadTexture(blankIcon);
}

void DrawInventory(InventoryDatabase* inventory) {
    int startX = (WINDOW_WIDTH - (SLOTS_PER_ROW * (SLOT_SIZE + INVENTORY_PADDING))) / 2;
    int startY = (WINDOW_HEIGHT - ((TABLE_SIZE / SLOTS_PER_ROW) * (SLOT_SIZE + INVENTORY_PADDING))) / 2;

    // Draw inventory background
    DrawRectangle(startX - INVENTORY_PADDING,
                  startY - INVENTORY_PADDING,
                  (SLOT_SIZE + INVENTORY_PADDING) * SLOTS_PER_ROW + INVENTORY_PADDING,
                  (SLOT_SIZE + INVENTORY_PADDING) * (TABLE_SIZE / SLOTS_PER_ROW) + INVENTORY_PADDING,
                  DARKGRAY);

    // Draw inventory slots
    InventoryNode* current = inventory->head;

    for (int row = 0; row < TABLE_SIZE / SLOTS_PER_ROW; row++) {
        for (int col = 0; col < SLOTS_PER_ROW; col++) {
            int x = startX + col * (SLOT_SIZE + INVENTORY_PADDING);
            int y = startY + row * (SLOT_SIZE + INVENTORY_PADDING);

            // Draw slot background
            DrawRectangle(x, y, SLOT_SIZE, SLOT_SIZE, LIGHTGRAY);

            // If we have an item to draw
            if (current != NULL) {
                // Find the corresponding enum value for the item
                int itemEnum = -1;
                for (int i = 0; i < 10; i++) {
                    if (strcmp(current->name, items[i].name) == 0) {
                        itemEnum = i;
                        break;
                    }
                }

                if (itemEnum != -1) {
                    DrawTexture(itemIcons[itemEnum].iconTexture, x, y, WHITE);
                    // Draw quantity if greater than 1
                    if (current->quantity > 1) {
                        DrawText(TextFormat("%d", current->quantity),
                                 x + SLOT_SIZE - 20, y + SLOT_SIZE - 20, 20, WHITE);
                    }
                }
                current = current->next;
            } else {
                // Draw blank icon for empty slots
                DrawTexture(blankIcon, x, y, WHITE);
            }
        }
    }
}


int main() {
    // Initialize window
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Inventory System");
    SetTargetFPS(60);

    // Initialize inventory
    InventoryDatabase inventory;
    init_inventory_database(&inventory);

    // Load icons
    LoadItemIcons();

    // Add some items to test
    add_item_to_inventory(&inventory, &items[SWORD], 2);
    add_item_to_inventory(&inventory, &items[SHIELD], 1);
    add_item_to_inventory(&inventory, &items[BOW], 3);
    add_item_to_inventory(&inventory, &items[STAFF], 1);
    add_item_to_inventory(&inventory, &items[DAGGER], 5);

    // Main game loop
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawInventory(&inventory);

        EndDrawing();
    }

    // Cleanup
    UnloadItemIcons();
    CloseWindow();

    return 0;
}