#include "stdafx.h"
#include "Pendulum.h"
#include "../../../Include/xrRender/Kinematics.h"
#include "../../../Include/xrRender/RenderVisual.h"
#include "../../../xrEngine/CameraManager.h"
#include "../../Actor.h"
#include "../../ActorEffector.h"
#include "../../CameraFirstEye.h"
#include "../../HUDManager.h"
#include "../../Level.h"
#include "../../PHWorld.h"
#include "../../PhysicsShell.h"
#include "../../bone_callback_type.h"

CPendulum::CPendulum() = default;
CPendulum::~CPendulum() = default;

void CPendulum::Load(LPCSTR section)
{
    CPhysicObject::Load(section);
    m_ticSnd.create("device\\clock_tic", st_Effect, sg_SourceType);
    m_tacSnd.create("device\\clock_tic", st_Effect, sg_SourceType);
    m_kukuSnd.create("device\\clock_kuku", st_Effect, sg_SourceType);
    m_kukuHalfSnd.create("device\\clock_kuku_half", st_Effect, sg_SourceType);
    m_openSnd.create("device\\wood_small_open", st_Effect, sg_SourceType);
    m_closeStartSnd.create("device\\wood_small_close_start", st_Effect, sg_SourceType);
    m_closeStopSnd.create("device\\wood_small_close_stop", st_Effect, sg_SourceType);
}

BOOL CPendulum::net_Spawn(CSE_Abstract* DC)
{
    if (!CPhysicObject::net_Spawn(DC))
        return false;

    m_prevGameTime = Level().GetGameTime();
    camera.reset(xr_new<CCameraFirstEye>(
        this, CCameraBase::flRelativeLink | CCameraBase::flPositionRigid | CCameraBase::flDirectionRigid));

    set_tip_text("clock_use");

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

    const u32 RANGE = 12 * 60 * 60;

    if (m_timeSpeed != 0) {
        m_time += RANGE + m_timeSpeed;
        if (m_time < 0)
            m_time += RANGE;
        if (m_time > RANGE)
            m_time -= RANGE;
    }

    auto now = Level().GetGameTime();
    u32 add = u32((now - m_prevGameTime) / 1000);
    m_prevGameTime = m_prevGameTime + add * 1000;

    if (m_working) {
        m_time += add;
        if (m_time >= RANGE)
            m_time -= RANGE;
    }

    m_hours = m_time / float(60 * 60);
    m_minutes = (m_time % (60 * 60)) / float(60);

    if (!m_working)
        return;

    Fvector v;
    m_pendulum->get_LinearVel(v);
    bool clockwise = v.x > 0.f;
    if (m_prevPendulumClockwise != clockwise && Device.dwTimeGlobal - m_lastImpulse > 250) {
        auto&& m = m_pendulum->mXFORM;
        ref_sound* tictac = nullptr;
        if (clockwise) {
            if (m_working) {
                m_pendulum->applyImpulse({ 1, 1, 1 }, 4);
            }
            tictac = &m_ticSnd;
        } else {
            tictac = &m_tacSnd;
        }
        tictac->play_at_pos(this, m.c, 0, 0);
        tictac->set_volume(1.0f);
        m_prevPendulumClockwise = clockwise;
        m_lastImpulse = Device.dwTimeGlobal;
    }

    u32 half = m_time / (30 * 60);
    if (half != m_prevHalf) {
        auto&& m = XFORM();
        ref_sound* kuku = half % 2 ? &m_kukuHalfSnd : &m_kukuSnd;
        kuku->play_at_pos(this, m.c, 0, 0);
        kuku->set_volume(1.0f);
        m_prevHalf = half;
    }

    if (m_doorClosing) {
        auto joint = PPhysicsShellHolder()->PPhysicsShell()->get_Joint("door");
        if (std::abs(joint->GetAxisAngle(0)) < 0.01) {
            m_closeStopSnd.play_at_pos(this, XFORM().c, 0, 0);
            m_closeStopSnd.set_volume(1.0f);
            m_doorClosing = false;
        }
    }

    // почему-то он отключается через какое-то время
    m_pendulum->Enable();
}

void CPendulum::renderable_Render()
{
    CPhysicObject::renderable_Render();

    if (OwnerActor()) {
        HUD().Font().pFontLetterica18Russian->SetColor(0xffffffff);
        HUD().Font().pFontLetterica18Russian->SetAligment(CGameFont::alLeft);
        HUD().Font().pFontLetterica18Russian->OutSetI(0.6, 0.7);
        HUD().Font().pFontLetterica18Russian->OutNext("Перевести вперед часовую стрелку - W");
        HUD().Font().pFontLetterica18Russian->OutNext("Перевести назад часовую стрелку - S");
        HUD().Font().pFontLetterica18Russian->OutNext("Перевести вперед минутную стрелку - D");
        HUD().Font().pFontLetterica18Russian->OutNext("Перевести назад минутную стрелку - A");
        HUD().Font().pFontLetterica18Russian->OutNext("Остановить/запустить часы - SPACE");
        HUD().Font().pFontLetterica18Russian->OutNext("Отойти - F");
    }
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

////////////////////////////////////////////////////////////////////////////////
// CHolderCustom overrides
////////////////////////////////////////////////////////////////////////////////

void CPendulum::OnMouseMove(int x, int y) { }

void CPendulum::OnKeyboardPress(int dik)
{
    switch (dik) {
    case kFWD:
        m_timeSpeed = 120;
        break;
    case kBACK:
        m_timeSpeed = -120;
        break;
    case kL_STRAFE:
        m_timeSpeed = -15;
        break;
    case kR_STRAFE:
        m_timeSpeed = 15;
        break;
    case kJUMP:
        m_working = !m_working;
        if (m_working) {
            m_pendulum->applyImpulse({ 1, 1, 1 }, 100);
        }
        break;
    }
}

void CPendulum::OnKeyboardRelease(int dik)
{
    switch (dik) {
    case kFWD:
    case kBACK:
    case kL_STRAFE:
    case kR_STRAFE:
        m_timeSpeed = 0;
        break;
    }
}

void CPendulum::OnKeyboardHold(int dik) { }

CInventory* CPendulum::GetInventory() { return nullptr; }

void CPendulum::cam_Update(float dt, float fov)
{
    IKinematics* K = smart_cast<IKinematics*>(Visual());
    K->CalculateBones_Invalidate();
    K->CalculateBones(TRUE);
    Fmatrix C = K->LL_GetTransform(K->LL_BoneID("use_place"));
    Fvector P {}, Da {};
    XFORM().transform_tiny(P, C.c);
    Camera()->Update(P, Da);
    Level().Cameras().UpdateFromCamera(Camera());
}

bool CPendulum::Use(const Fvector& pos, const Fvector& dir, const Fvector& foot_pos)
{
    // если прикреплены к часам, то всегда можно открепиться
    if (Owner())
        return true;

    auto model = Visual()->dcast_PKinematics();
    const float OPENED_ANGLE = -M_PI / 2.f;

    // если смотрим на дверцу, то управляем дверцей(открываем/закрываем)
    collide::rq_results RQR;
    collide::ray_defs Q(pos, dir, 3.f, CDB::OPT_ONLYNEAREST, collide::rqtObject);
    VERIFY(!fis_zero(Q.dir.square_magnitude()));
    if (g_pGameLevel->ObjectSpace.RayQuery(RQR, collidable.model, Q)) {
        auto bid = (u16)RQR.r_begin()->element;
        if (model->LL_GetData(bid).name == "door") {
            auto joint = PPhysicsShellHolder()->PPhysicsShell()->get_Joint(bid);
            bool opened = joint->GetAxisAngle(0) < OPENED_ANGLE;
            joint->SetForceAndVelocity(1000.f * (opened ? 1 : -1), 3.f * (opened ? 1 : -1));
            ref_sound* snd = opened ? &m_closeStartSnd : &m_openSnd;
            snd->play_at_pos(this, XFORM().c, 0, 0);
            snd->set_volume(1.0f);
            if (opened)
                m_doorClosing = true;
            return false;
        }
    }

    // проверим, что дверца полностью открыта и прикрепимся
    auto joint = PPhysicsShellHolder()->PPhysicsShell()->get_Joint("door");
    bool opened = joint->GetAxisAngle(0) < OPENED_ANGLE;
    if (opened)
        m_exitPosition = foot_pos;
    return opened;
}

bool CPendulum::allowWeapon() const { return true; }

bool CPendulum::HUDView() const { return true; }

Fvector CPendulum::ExitPosition() { return m_exitPosition; }

CCameraBase* CPendulum::Camera() { return camera.get(); }

void CPendulum::UpdateEx(float fov)
{
    if (OwnerActor() && OwnerActor()->IsMyCamera()) {
        cam_Update(Device.fTimeDelta, fov);
        OwnerActor()->Orientation().yaw = -camera->yaw;
        OwnerActor()->Orientation().pitch = -camera->pitch;
        OwnerActor()->Cameras().UpdateFromCamera(Camera());
        OwnerActor()->Cameras().ApplyDevice(VIEWPORT_NEAR);
    }
}