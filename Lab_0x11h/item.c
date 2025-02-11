#include "item.h"

Item items[10];

void InitItems(void){
    items[0] = (Item){
        .name = "Sword",
        .texture = LoadTexture("Icons/sword.png"),
        .value = 100,
        .rarity = COMMON,
        .weight = 2.5
    };
    items[1] = (Item){
        .name = "Shield",
        .texture = LoadTexture("Icons/shield.png"),
        .value = 150,
        .rarity = UNCOMMON,
        .weight = 5.0
    };
    items[2] = (Item){
        .name = "Bow",
        .texture = LoadTexture("Icons/bow.png"),
        .value = 200,
        .rarity = RARE,
        .weight = 1.5
    };
    items[3] = (Item){
        .name = "Axe",
        .texture = LoadTexture("Icons/axe.png"),
        .value = 250,
        .rarity = EPIC,
        .weight = 3.0
    };
    items[4] = (Item){
        .name = "Staff",
        .texture = LoadTexture("Icons/staff.png"),
        .value = 300,
        .rarity = LEGENDARY,
        .weight = 2.0
    };
    items[5] = (Item){
        .name = "Dagger",
        .texture = LoadTexture("Icons/dagger.png"),
        .value = 50,
        .rarity = COMMON,
        .weight = 1.0
    };
    items[6] = (Item){
        .name = "Mace",
        .texture = LoadTexture("Icons/mace.png"),
        .value = 75,
        .rarity = UNCOMMON,
        .weight = 3.5
    };
    items[7] = (Item){
        .name = "GreatAxe",
        .texture = LoadTexture("Icons/greataxe.png"),
        .value = 125,
        .rarity = RARE,
        .weight = 4.0
    };
    items[8] = (Item){
        .name = "Crossbow",
        .texture = LoadTexture("Icons/crossbow.png"),
        .value = 175,
        .rarity = EPIC,
        .weight = 2.5
    };
    items[9] = (Item){
        .name = "Cloak",
        .texture = LoadTexture("Icons/cloak.png"),
        .value = 225,
        .rarity = LEGENDARY,
        .weight = 1.0
    };
}

void CleanupItems(void) {
    // Unload textures to avoid memory leaks
    for (int i = 0; i < 10; i++) {
        UnloadTexture(items[i].texture);
    }
}