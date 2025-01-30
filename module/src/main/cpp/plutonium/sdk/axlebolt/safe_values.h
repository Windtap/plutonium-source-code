struct safe_float
{
    int f1;
    int f2;

    inline float get_value()
    {
        return (float) (f2 ^ f1);
    }

    inline void set_value(float value)
    {
        f2 = (int) value ^ f1;
    }
};