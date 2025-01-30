struct UnityEngine_Touch_Fields
{
    int32_t m_FingerId;
    unity::structures::Vector2 m_Position;
    unity::structures::Vector2 m_RawPosition;
    unity::structures::Vector2 m_PositionDelta;
    float m_TimeDelta;
    int32_t m_TapCount;
    int32_t m_Phase;
    int32_t m_Type;
    float m_Pressure;
    float m_maximumPossiblePressure;
    float m_Radius;
    float m_RadiusVariance;
    float m_AltitudeAngle;
    float m_AzimuthAngle;
};

enum TouchPhase
{
    Began = 0,
    Moved = 1,
    Stationary = 2,
    Ended = 3,
    Canceled = 4
};

int (*Input$$get_touchCount)();
int _Input$$get_touchCount()
{
    static uintptr_t il2cpp_addr = _g_cheat.load()->_il2cpp->address();
    static void *GetTouchPtr = (void *) (il2cpp_addr + offsets::Input$$GetTouch);

    ImGuiIO &io = ImGui::GetIO();
    if (GetTouchPtr)
    {
        int touchCount = Input$$get_touchCount();// public static int get_touchCount() { }
        if (touchCount > 0)
        {
            UnityEngine_Touch_Fields touch = ((UnityEngine_Touch_Fields (*)(int))GetTouchPtr)(0); // public static Touch GetTouch(int index) { }
            float reverseY = io.DisplaySize.y - touch.m_Position.y;
            switch (touch.m_Phase)
            {
                case TouchPhase::Began:
                case TouchPhase::Stationary:
                    io.MousePos = ImVec2(touch.m_Position.x, reverseY);
                    io.MouseDown[0] = true;
                    break;
                case TouchPhase::Ended:
                case TouchPhase::Canceled:
                    io.MouseDown[0] = false;
                    break;
                case TouchPhase::Moved:
                    io.MousePos = ImVec2(touch.m_Position.x, reverseY);
                    break;
                default:
                    break;
            }
        }
    }

    if (io.WantCaptureMouse) { return 0; }

    return Input$$get_touchCount();
}
