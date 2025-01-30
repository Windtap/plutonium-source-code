struct snapshots
{

};

struct movement_snapshot
{
    il2cpp::klass *klass;
    void *monitor;
};

struct mecanim_snapshot
{
    il2cpp::klass *klass;
    void *monitor;
};

struct aim_snapshot
{
    il2cpp::klass *klass;
    void *monitor;
};

struct weapon_slot_item
{
    il2cpp::klass *klass;
    void *monitor;

    int owner;
    int weapon_id;
    int skin_id;
    uint8_t slot;
};

struct weaponry_snapshot
{
    il2cpp::klass *klass;
    void *monitor;

    uint8_t current_slot;
    void *weapon_snapshot;
    int weapon_type;
    int weapon_id;
    il2cpp::list<weapon_slot_item*> *items;
};

struct player_snapshot
{
    il2cpp::klass *klass;
    void *monitor;

    void *unk;
    snapshots *sn;
    movement_snapshot *movement;
    mecanim_snapshot *mecanim;
    aim_snapshot *aim;
    weaponry_snapshot *weaponry;


    static void (*serialize)(void *, void *);
};

void (*player_snapshot::serialize)(void *, void *){};