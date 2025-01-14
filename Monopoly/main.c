#include <string.h>

typedef enum{ false, true } bool;
typedef enum{ Brown, Light_Blue, Pink, Orange, Red, Yellow, Green, Blue } Color;
typedef enum{ Community_Chest, Chance_Card } Card_Type;
typedef enum {
    TILE_GO,
    TILE_PROPERTY,
    TILE_RAILROAD,
    TILE_UTILITY,
    TILE_TAX,
    TILE_CHANCE,
    TILE_COMMUNITY_CHEST,
    TILE_JAIL,
    TILE_GO_TO_JAIL,
    TILE_FREE_PARKING,
    TILE_JUST_VISITING
} TileType;

typedef struct {
    int Price;
    char Name[32];
    bool Is_Owned;
    Color Tile_Color;
    int Rent;
    int Rent_With_Houses[4];
    int Rent_With_Hotel;
    int House_Cost;
    int Num_Houses;
    bool Has_Hotel;
    int Mortgage_Value;
    bool Is_Mortgaged;
    int Owner_PlayerID;
} Properties;

typedef struct
{
    int Price;
    char Name[32];
} Railroads;

typedef struct {
    int Price;
    char Name[32];
    bool Is_Owned;
} Utilities;

typedef struct
{
    Card_Type cardType;
    char Name[32];
    char Card_Text[128];
} Cards;

typedef struct {
    TileType type;
    char name[32];
    union {
        Properties* property;
        Railroads* railroad;
        Utilities* utility;
        int taxAmount;
    } data;
} Tile;

typedef struct {
    Tile tiles[40];
    Properties properties[22];
    Railroads railroads[4];
    Utilities utilities[2];
    Cards communityChest[16];
    Cards chanceCard[16];
    int Free_Parking_Money;
    int Available_Houses;
    int Available_Hotels;
} Board;

typedef struct {
    int Player_Position;
    int Player_Money;
    int Player_Owned_Properties[22];
    int Player_Owned_Railroads[4];
    int Player_Owned_Utilities[2];
    int Player_Number;
    int Turns_In_Jail;
    int Get_Out_Of_Jail_Cards;
    bool In_Jail;
    bool isPlaying;
    bool isBankrupt;
} Player;

typedef struct {
    int Current_Player;
    int Dice_Roll1;
    int Dice_Roll2;
    int Doubles_Count;
    bool Game_Over;
} GameState;

void initializePlayers(Player *players, int numPlayers) {
    for (int i = 0; i < numPlayers; i++)
    {
        Player *player = &players[i];

        player->Player_Position = 0;
        player->Player_Money = 1500;
        memset(player->Player_Owned_Properties, 0, sizeof(player->Player_Owned_Properties));
        memset(player->Player_Owned_Railroads, 0, sizeof(player->Player_Owned_Railroads));
        memset(player->Player_Owned_Utilities, 0, sizeof(player->Player_Owned_Utilities));
        player->Player_Number = i + 1;
        player->Turns_In_Jail = 0;
        player->Get_Out_Of_Jail_Cards = 0;
        player->In_Jail = false;
        player->isPlaying = true;
        player->isBankrupt = false;
    }
}
void initializeBoard(Board *board) {

    Properties properties[] = {
            {60,  "Mediterranean Ave",  false, Brown,      2,  {10,  30,  90,   160},  250,  50,  0, false, 30,  false, -1},
            {60,  "Baltic Ave",         false, Brown,      4,  {20,  60,  180,  320},  450,  50,  0, false, 30,  false, -1},
            {100, "Oriental Ave",       false, Light_Blue, 6,  {30,  90,  270,  400},  550,  50,  0, false, 50,  false, -1},
            {100, "Vermont Ave",        false, Light_Blue, 6,  {30,  90,  270,  400},  550,  50,  0, false, 50,  false, -1},
            {120, "Connecticut Ave",    false, Light_Blue, 8,  {40,  100, 300,  450},  600,  50,  0, false, 60,  false, -1},
            {140, "St. Charles Place",  false, Pink,       10, {50,  150, 450,  625},  750,  100, 0, false, 70,  false, -1},
            {140, "States Ave",         false, Pink,       10, {50,  150, 450,  625},  750,  100, 0, false, 70,  false, -1},
            {160, "Virginia Ave",       false, Pink,       12, {60,  180, 500,  700},  900,  100, 0, false, 80,  false, -1},
            {180, "St. James Place",    false, Orange,     14, {70,  200, 550,  750},  950,  100, 0, false, 90,  false, -1},
            {180, "Tennessee Ave",      false, Orange,     14, {70,  200, 550,  750},  950,  100, 0, false, 90,  false, -1},
            {200, "New York Ave",       false, Orange,     16, {80,  220, 600,  800},  1000, 100, 0, false, 100, false, -1},
            {220, "Kentucky Ave",       false, Red,        18, {90,  250, 700,  875},  1050, 150, 0, false, 110, false, -1},
            {220, "Indiana Ave",        false, Red,        18, {90,  250, 700,  875},  1050, 150, 0, false, 110, false, -1},
            {240, "Illinois Ave",       false, Red,        20, {100, 300, 750,  925},  1100, 150, 0, false, 120, false, -1},
            {260, "Atlantic Ave",       false, Yellow,     22, {110, 330, 800,  975},  1150, 150, 0, false, 130, false, -1},
            {260, "Ventnor Ave",        false, Yellow,     22, {110, 330, 800,  975},  1150, 150, 0, false, 130, false, -1},
            {280, "Marvin Gardens",     false, Yellow,     24, {120, 360, 850,  1025}, 1200, 150, 0, false, 140, false, -1},
            {300, "Pacific Ave",        false, Green,      26, {130, 390, 900,  1100}, 1275, 200, 0, false, 150, false, -1},
            {300, "North Carolina Ave", false, Green,      26, {130, 390, 900,  1100}, 1275, 200, 0, false, 150, false, -1},
            {320, "Pennsylvania Ave",   false, Green,      28, {150, 450, 1000, 1200}, 1400, 200, 0, false, 160, false, -1},
            {350, "Park Place",         false, Blue,       35, {175, 500, 1100, 1300}, 1500, 200, 0, false, 175, false, -1},
            {400, "Boardwalk",          false, Blue,       50, {200, 600, 1400, 1700}, 2000, 200, 0, false, 200, false, -1}
    };
    memcpy(board->properties, properties, sizeof(properties));

    // Initialize railroads
    Railroads railroads[] = {
            {200, "Reading Railroad"},
            {200, "Pennsylvania RR"},
            {200, "B. & O. Railroad"},
            {200, "Short Line"}
    };
    memcpy(board->railroads, railroads, sizeof(railroads));

    // Initialize utilities
    Utilities utilities[] = {
            {150, "Electric Company", false},
            {150, "Water Works",      false}
    };
    memcpy(board->utilities, utilities, sizeof(utilities));

    // GO
    board->tiles[0].type = TILE_GO;
    strcpy(board->tiles[0].name, "GO");

    // Brown properties
    {
        board->tiles[1].type = TILE_PROPERTY;
        strcpy(board->tiles[1].name, "Mediterranean Avenue");
        board->tiles[1].data.property = &board->properties[0];

        board->tiles[2].type = TILE_COMMUNITY_CHEST;
        strcpy(board->tiles[2].name, "Community Chest");

        board->tiles[3].type = TILE_PROPERTY;
        strcpy(board->tiles[3].name, "Baltic Avenue");
        board->tiles[3].data.property = &board->properties[1];

        board->tiles[4].type = TILE_TAX;
        strcpy(board->tiles[4].name, "Income Tax");
        board->tiles[4].data.taxAmount = 200;

        board->tiles[5].type = TILE_RAILROAD;
        strcpy(board->tiles[5].name, "Reading Railroad");
        board->tiles[5].data.railroad = &board->railroads[0];
    }
    // Light Blue properties
    {
        board->tiles[6].type = TILE_PROPERTY;
        strcpy(board->tiles[6].name, "Oriental Avenue");
        board->tiles[6].data.property = &board->properties[2];

        board->tiles[7].type = TILE_CHANCE;
        strcpy(board->tiles[7].name, "Chance");

        board->tiles[8].type = TILE_PROPERTY;
        strcpy(board->tiles[8].name, "Vermont Avenue");
        board->tiles[8].data.property = &board->properties[3];

        board->tiles[9].type = TILE_PROPERTY;
        strcpy(board->tiles[9].name, "Connecticut Avenue");
        board->tiles[9].data.property = &board->properties[4];

        board->tiles[10].type = TILE_JAIL;
        strcpy(board->tiles[10].name, "Jail");
    }
    // Pink properties
    {
        board->tiles[11].type = TILE_PROPERTY;
        strcpy(board->tiles[11].name, "St. Charles Place");
        board->tiles[11].data.property = &board->properties[5];

        board->tiles[12].type = TILE_UTILITY;
        strcpy(board->tiles[12].name, "Electric Company");
        board->tiles[12].data.utility = &board->utilities[0];

        board->tiles[13].type = TILE_PROPERTY;
        strcpy(board->tiles[13].name, "States Avenue");
        board->tiles[13].data.property = &board->properties[6];

        board->tiles[14].type = TILE_PROPERTY;
        strcpy(board->tiles[14].name, "Virginia Avenue");
        board->tiles[14].data.property = &board->properties[7];

        board->tiles[15].type = TILE_RAILROAD;
        strcpy(board->tiles[15].name, "Pennsylvania Railroad");
        board->tiles[15].data.railroad = &board->railroads[1];
    }
    // Orange properties
    {
        board->tiles[16].type = TILE_PROPERTY;
        strcpy(board->tiles[16].name, "St. James Place");
        board->tiles[16].data.property = &board->properties[8];

        board->tiles[17].type = TILE_COMMUNITY_CHEST;
        strcpy(board->tiles[17].name, "Community Chest");

        board->tiles[18].type = TILE_PROPERTY;
        strcpy(board->tiles[18].name, "Tennessee Avenue");
        board->tiles[18].data.property = &board->properties[9];

        board->tiles[19].type = TILE_PROPERTY;
        strcpy(board->tiles[19].name, "New York Avenue");
        board->tiles[19].data.property = &board->properties[10];

        board->tiles[20].type = TILE_FREE_PARKING;
        strcpy(board->tiles[20].name, "Free Parking");
    }
    // Red properties
    {
        board->tiles[21].type = TILE_PROPERTY;
        strcpy(board->tiles[21].name, "Kentucky Avenue");
        board->tiles[21].data.property = &board->properties[11];

        board->tiles[22].type = TILE_CHANCE;
        strcpy(board->tiles[22].name, "Chance");

        board->tiles[23].type = TILE_PROPERTY;
        strcpy(board->tiles[23].name, "Indiana Avenue");
        board->tiles[23].data.property = &board->properties[12];

        board->tiles[24].type = TILE_PROPERTY;
        strcpy(board->tiles[24].name, "Illinois Avenue");
        board->tiles[24].data.property = &board->properties[13];

        board->tiles[25].type = TILE_RAILROAD;
        strcpy(board->tiles[25].name, "B. & O. Railroad");
        board->tiles[25].data.railroad = &board->railroads[2];
    }
    // Yellow properties
    {
        board->tiles[26].type = TILE_PROPERTY;
        strcpy(board->tiles[26].name, "Atlantic Avenue");
        board->tiles[26].data.property = &board->properties[14];

        board->tiles[27].type = TILE_PROPERTY;
        strcpy(board->tiles[27].name, "Ventnor Avenue");
        board->tiles[27].data.property = &board->properties[15];

        board->tiles[28].type = TILE_UTILITY;
        strcpy(board->tiles[28].name, "Water Works");
        board->tiles[28].data.utility = &board->utilities[1];

        board->tiles[29].type = TILE_PROPERTY;
        strcpy(board->tiles[29].name, "Marvin Gardens");
        board->tiles[29].data.property = &board->properties[16];

        board->tiles[30].type = TILE_GO_TO_JAIL;
        strcpy(board->tiles[30].name, "Go To Jail");
    }
    // Green properties
    {
        board->tiles[31].type = TILE_PROPERTY;
        strcpy(board->tiles[31].name, "Pacific Avenue");
        board->tiles[31].data.property = &board->properties[17];

        board->tiles[32].type = TILE_PROPERTY;
        strcpy(board->tiles[32].name, "North Carolina Avenue");
        board->tiles[32].data.property = &board->properties[18];

        board->tiles[33].type = TILE_COMMUNITY_CHEST;
        strcpy(board->tiles[33].name, "Community Chest");

        board->tiles[34].type = TILE_PROPERTY;
        strcpy(board->tiles[34].name, "Pennsylvania Avenue");
        board->tiles[34].data.property = &board->properties[19];

        board->tiles[35].type = TILE_RAILROAD;
        strcpy(board->tiles[35].name, "Short Line");
        board->tiles[35].data.railroad = &board->railroads[3];

        board->tiles[36].type = TILE_CHANCE;
        strcpy(board->tiles[36].name, "Chance");
    }
}

int main()
{
    Board Game_Board;
    Player Players[8];
    int numberOfPlayers = 4;

    initializeBoard(&Game_Board);

    initializePlayers(Players, numberOfPlayers);

    return 0;
}