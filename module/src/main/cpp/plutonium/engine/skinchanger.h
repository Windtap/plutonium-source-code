struct skin_t
{
    int id;
    int definition_id;


};

class skinchanger_t
{
private:
    static skinchanger_t *instance;

    skin_t skins;
};