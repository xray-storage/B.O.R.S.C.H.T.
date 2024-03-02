#include "stdafx.h"
#include "Pendulum.h"
#include "../../../Include/xrRender/Kinematics.h"
#include "../../../Include/xrRender/RenderVisual.h"
#include "../../Level.h"
#include "../../PhysicsShell.h"
#include "../../bone_callback_type.h"

void CPendulum::Load(LPCSTR section)
{
    CPhysicObject::Load(section);
    m_ticSnd.create("device\\clock_tic", st_Effect, sg_SourceType);
    m_tacSnd.create("device\\clock_tic", st_Effect, sg_SourceType);
    m_kukuSnd.create("device\\clock_kuku", st_Effect, sg_SourceType);
    m_kukuHalfSnd.create("device\\clock_kuku_half", st_Effect, sg_SourceType);
}

BOOL CPendulum::net_Spawn(CSE_Abstract* DC)
{
    if (!CPhysicObject::net_Spawn(DC))
        return false;

    auto model = Visual()->dcast_PKinematics();
    auto bid = model->LL_BoneID("hour_hand");
    model->LL_GetBoneInstance(bid).set_callback(bctHour, BoneCallback, this);
    bid = model->LL_BoneID("minute_hand");
    model->LL_GetBoneInstance(bid).set_callback(bctMinute, BoneCallback, this);
    auto joint = PPhysicsShellHolder()->PPhysicsShell()->get_Joint("pendulum");
    joint->SetJointSDfactors(0.f, 0.f);
    m_pendulum = PPhysicsShellHolder()->PPhysicsShell()->get_Element("pendulum");
    m_pendulum->SetAirResistance(0.f, 0.f);
    m_pendulum->applyImpulse({ 1, 1, 1 }, 100);

    SheduleRegister();

    return true;
}

void CPendulum::shedule_Update(u32 dt)
{
    CPhysicObject::shedule_Update(dt);

    auto gt = Level().GetGameTime();
    u32 time = gt % (12 * 60 * 60 * 1000);
    m_hours = time / float(60 * 60 * 1000);
    time = time % (60 * 60 * 1000);
    m_minutes = time / float(60 * 1000);

    Fvector v;
    m_pendulum->get_LinearVel(v);
    bool clockwise = v.x > 0.f;
    if (m_prevPendulumClockwise != clockwise) {
        auto&& m = m_pendulum->mXFORM;
        ref_sound* tictac = nullptr;
        if (clockwise) {
            m_pendulum->applyImpulse({ 1, 1, 1 }, 4);
            tictac = &m_ticSnd;
        } else {
            tictac = &m_tacSnd;
        }
        tictac->play_at_pos(this, m.c, 0, 0);
        tictac->set_volume(1.0f);
        m_prevPendulumClockwise = clockwise;
    }

    u32 half = time / (30 * 60 * 1000);
    if (half != m_prevHalf) {
        auto&& m = XFORM();
        ref_sound* kuku = half % 2 ? &m_kukuHalfSnd : &m_kukuSnd;
        kuku->play_at_pos(this, m.c, 0, 0);
        kuku->set_volume(1.0f);
        m_prevHalf = half;
    }

    // почему-то он отключается через какое-то время
    m_pendulum->Enable();
}

void CPendulum::BoneCallback(CBoneInstance* B)
{
    CPendulum* self = static_cast<CPendulum*>(B->callback_param());
    float angle = 0.f;
    if (B->callback_type() == bctHour)
        angle = self->m_hours / 12.f * PI_MUL_2;
    else if (B->callback_type() == bctMinute)
        angle = self->m_minutes / 60.f * PI_MUL_2;

    Fmatrix rY;
    rY.rotateZ(-angle);
    B->mTransform.mulB_43(rY);
}