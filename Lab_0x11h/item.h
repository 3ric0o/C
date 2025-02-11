#ifndef LAB_0X11H_ITEM_H
#define LAB_0X11H_ITEM_H

#define MAX_ITEM_NAME 32

enum Rarity {
    COMMON,
    UNCOMMON,
    RARE,
    EPIC,
    LEGENDARY
};

typedef struct {
    char name[MAX_ITEM_NAME];
    int value;
    enum Rarity rarity;
    float weight;
} Item;

// Global items array
extern Item items[10];

#endif //LAB_0X11H_ITEM_H