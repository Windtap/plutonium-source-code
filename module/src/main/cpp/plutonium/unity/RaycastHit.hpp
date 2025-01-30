#pragma once

#include "Vector3.hpp"
#include "Vector2.hpp"

namespace unity::structures {
    struct RaycastHit {
        Vector3 point{}, normal{};
        unsigned int faceID{};
        float distance{};
        Vector2 UV{};
#if UNITY_VER > 174
        int m_Collider{};
#else
        void *m_Collider{};
#endif
        void *GetCollider() const;
    };
}
