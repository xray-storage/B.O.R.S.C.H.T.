#include "stdafx.h"
#include "../xrEngine/CameraBase.h"
#include "ActorEffector.h"
#include "actor.h"
#include "holder_custom.h"
#ifdef DEBUG
#    include "PHDebug.h"
#endif
#include "../Include/xrRender/Kinematics.h"
#include "Car.h"
#include "HudItem.h"
#include "PHDestroyable.h"
#include "PHShellSplitter.h"
#include "Torch.h"
#include "actor_anim_defs.h"
#include "alife_space.h"
#include "bone_callback_type.h"
#include "characterphysicssupport.h"
#include "clsid_game.h"
#include "game_object_space.h"
#include "hit.h"
#include "inventory.h"

void CActor::attach_Holder(CHolderCustom* vehicle)
{
    if (!vehicle)
        return;

    if (m_holder)
        return;
    PickupModeOff();
    m_holder = vehicle;

    auto V = Visual()->dcast_PKinematicsAnimated();
    auto pK = Visual()->dcast_PKinematics();
    ;

    if (!m_holder->attach_Actor(this)) {
        m_holder = NULL;
        return;
    }
    // temp play animation
    CCar* car = smart_cast<CCar*>(m_holder);
    if (car) {
        u16 anim_type = car->DriverAnimationType();
        SVehicleAnimCollection& anims = m_vehicle_anims->m_vehicles_type_collections[anim_type];
        V->PlayCycle(anims.idles[0], FALSE);
    }

    ResetCallbacks();
    u16 head_bone = pK->LL_BoneID("bip01_head");
    pK->LL_GetBoneInstance(u16(head_bone)).set_callback(bctPhysics, VehicleHeadCallback, this);

    character_physics_support()->movement()->DestroyCharacter();
    mstate_wishful = 0;
    m_holderID = m_holder->cast_game_object()->ID();

    SetWeaponHideState(INV_STATE_CAR, true);

    CStepManager::on_animation_start(MotionID(), 0);
}

void CActor::detach_Holder()
{
    if (!m_holder)
        return;

    auto car = m_holder->cast_game_object()->cast_physics_shell_holder();
    if (!car)
        return;
    CPHShellSplitterHolder* sh = car->PPhysicsShell()->SplitterHolder();
    if (sh)
        sh->Deactivate();
    if (!character_physics_support()->movement()->ActivateBoxDynamic(0)) {
        if (sh)
            sh->Activate();
        return;
    }
    if (sh)
        sh->Activate();
    m_holder->detach_Actor();

    character_physics_support()->movement()->SetPosition(m_holder->ExitPosition());
    character_physics_support()->movement()->SetVelocity(m_holder->ExitVelocity());

    r_model_yaw = -m_holder->Camera()->yaw;
    r_torso.yaw = r_model_yaw;
    r_model_yaw_dest = r_model_yaw;
    m_holder = NULL;
    SetCallbacks();
    auto V = Visual()->dcast_PKinematicsAnimated();
    R_ASSERT(V);
    V->PlayCycle(m_anims->m_normal.legs_idle);
    V->PlayCycle(m_anims->m_normal.m_torso_idle);
    m_holderID = u16(-1);

    SetWeaponHideState(INV_STATE_CAR, false);
}

void CActor::on_requested_spawn(CObject* object)
{
    CCar* car = smart_cast<CCar*>(object);
    attach_Holder(car);
}

bool CActor::use_Holder(CHolderCustom* holder)
{
    Fvector foot = Position();
    bool b = false;
    if (m_holder) {
        if (m_holder->Use(Device.vCameraPosition, Device.vCameraDirection, foot))
            detach_Holder();
        b = true;
    } else {
        if (holder) {
            if (holder->Use(Device.vCameraPosition, Device.vCameraDirection, foot)) {
                if (pCamBobbing) {
                    Cameras().RemoveCamEffector(eCEBobbing);
                    pCamBobbing = NULL;
                }

                attach_Holder(holder);

                // switch off torch...
                CAttachableItem* I = CAttachmentOwner::attachedItem(CLSID_DEVICE_TORCH);
                if (I) {
                    CTorch* torch = smart_cast<CTorch*>(I);
                    if (torch)
                        torch->Switch(false);
                }
            }
            b = true;
        }
    }

    if (inventory().ActiveItem()) {
        CHudItem* hi = smart_cast<CHudItem*>(inventory().ActiveItem());
        if (hi)
            hi->OnAnimationEnd(hi->GetState());
    }

    return b;
}