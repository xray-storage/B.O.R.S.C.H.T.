#pragma once

#include "../../PhysicObject.h"
#include "../../holder_custom.h"
#include <memory>

class CPendulum : public CPhysicObject, public CHolderCustom
{
public:
    CPendulum();
    ~CPendulum() override;

    // CObject overrides

    void Load(LPCSTR section) override;
    BOOL net_Spawn(CSE_Abstract* DC) override;
    void shedule_Update(u32 dt) override;
    void renderable_Render() override;

    // CHolderCustom overrides

    void UpdateEx(float fov) override;
    void OnMouseMove(int x, int y) override;
    void OnKeyboardPress(int dik) override;
    void OnKeyboardRelease(int dik) override;
    void OnKeyboardHold(int dik) override;
    CInventory* GetInventory() override;
    void cam_Update(float dt, float fov) override;
    bool Use(const Fvector& pos, const Fvector& dir, const Fvector& foot_pos) override;
    bool allowWeapon() const override;
    bool HUDView() const override;
    Fvector ExitPosition() override;
    CCameraBase* Camera() override;
    CGameObject* cast_game_object() override { return this; }

private:
    float m_hours = 0.f;
    float m_minutes = 0.f;
    CPhysicsElement* m_pendulum = nullptr;
    bool m_prevPendulumClockwise = true;
    ref_sound m_ticSnd, m_tacSnd, m_kukuSnd, m_kukuHalfSnd, m_openSnd, m_closeStartSnd, m_closeStopSnd;
    u32 m_prevHalf = 25;
    std::unique_ptr<CCameraBase> camera;
    Fvector m_exitPosition;
    u64 m_prevGameTime = 0;
    int m_time=0;
    int m_timeSpeed = 0;
    bool m_working = true;
    bool m_doorClosing = false;
    u32 m_lastImpulse = 0;

    static void BoneCallback(CBoneInstance* B);
};