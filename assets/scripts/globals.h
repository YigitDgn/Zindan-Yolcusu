#define ITEM_HEALTHELIXIR 3
#define MAX_DROPPED_ITEMS 20

typedef struct {
    float x, y;
    int type;
    bool active;
} DroppedItem;

extern DroppedItem droppedItems[MAX_DROPPED_ITEMS]; 