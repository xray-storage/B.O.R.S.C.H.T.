#pragma once

#include "../../PhysicObject.h"

class CPendulum : public CPhysicObject
{
public:
    void Load(LPCSTR section) override;
    BOOL net_Spawn(CSE_Abstract* DC) override;
    void shedule_Update(u32 dt) override;

private:
    float m_hours = 0.f;
    float m_minutes = 0.f;
    CPhysicsElement* m_pendulum = nullptr;
    bool m_prevPendulumClockwise = true;
    ref_sound m_ticSnd, m_tacSnd, m_kukuSnd, m_kukuHalfSnd;
    u32 m_prevHalf = 25;

    static void BoneCallback(CBoneInstance* B);
};