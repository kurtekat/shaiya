#pragma warning(disable: 28159) // GetTickCount
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shaiya/include/common/SConnection.h>
#include <shaiya/include/network/game/outgoing/0500.h>
#include <shaiya/include/skill/SkillAbilityType.h>
#include <util/util.h>
#include "include/main.h"
#include "include/shaiya/include/CGameData.h"
#include "include/shaiya/include/CSkill.h"
#include "include/shaiya/include/CUser.h"
#include "include/shaiya/include/CZone.h"
#include "include/shaiya/include/SkillInfo.h"
using namespace shaiya;

namespace user_apply_skill
{
    void ability_70_handler(CUser* user, SkillInfo* skillInfo)
    {
        SkillUseOutgoing outgoing{};
        outgoing.senderId = user->id;
        outgoing.targetId = user->id;
        outgoing.skillId = skillInfo->skillId;
        outgoing.skillLv = skillInfo->skillLv;

        if (!user->skillAbility70.triggered)
        {
            outgoing.statusType = SkillUseStatusType::Triggered;

            user->skillAbility70.triggered = true;
            user->skillAbility70.skillId = outgoing.skillId;
            user->skillAbility70.skillLv = outgoing.skillLv;
            user->skillAbility70.keepTime = GetTickCount() + (skillInfo->keepTime * 1000);

            SConnection::Send(&user->connection, &outgoing, sizeof(SkillUseOutgoing));
            CUser::AddApplySkillBuff(user, skillInfo);

            auto percentage = (user->health * skillInfo->abilities[0].value) / 100;
            user->health -= percentage;
            CUser::SendRecoverSet(user, user->health, user->stamina, user->mana);
        }
        else
        {
            outgoing.statusType = SkillUseStatusType::Stopped;

            user->skillAbility70.triggered = false;
            user->skillAbility70.skillId = 0;
            user->skillAbility70.skillLv = 0;
            user->skillAbility70.keepTime = 0;

            SConnection::Send(&user->connection, &outgoing, sizeof(SkillUseOutgoing));
            CUser::RemApplySkillBuff(user, skillInfo);
        }
    }

    void ability_70_update(CUser* user)
    {
        if (!user->skillAbility70.triggered)
            return;

        auto now = GetTickCount();
        if (now < user->skillAbility70.keepTime)
            return;

        auto skillInfo = CGameData::GetSkillInfo(user->skillAbility70.skillId, user->skillAbility70.skillLv);
        if (!skillInfo)
            return;

        auto percentage = (user->health * skillInfo->abilities[0].value) / 100;
        user->health -= percentage;
        CUser::SendRecoverSet(user, user->health, user->stamina, user->mana);

        user->skillAbility70.keepTime = now + (skillInfo->keepTime * 1000);
    }

    void ability_70_remove(CUser* user)
    {
        if (!user->skillAbility70.triggered)
            return;

        auto skillInfo = CGameData::GetSkillInfo(user->skillAbility70.skillId, user->skillAbility70.skillLv);
        if (!skillInfo)
            return;

        user->skillAbility70.triggered = false;
        user->skillAbility70.skillId = 0;
        user->skillAbility70.skillLv = 0;
        user->skillAbility70.keepTime = 0;

        CUser::RemApplySkillBuff(user, skillInfo);
    }

    void send_view(CUser* sender, CUser* target, SkillInfo* skillInfo, Packet buffer)
    {
        if (skillInfo->abilities[0].type == SkillAbilityType::Frenzied)
        {
            ability_70_handler(sender, skillInfo);
            return;
        }

        SkillUseOutgoing outgoing{};
        outgoing.targetType = util::deserialize<uint8_t>(buffer, 2);
        outgoing.senderId = sender->id;
        outgoing.targetId = target->id;
        outgoing.skillId = util::deserialize<uint16_t>(buffer, 11);
        outgoing.skillLv = util::deserialize<uint8_t>(buffer, 13);
        outgoing.health = util::deserialize<uint16_t>(buffer, 14);
        outgoing.stamina = util::deserialize<uint16_t>(buffer, 16);
        outgoing.mana = util::deserialize<uint16_t>(buffer, 18);

        if (!sender->zone)
            return;

        CZone::PSendView(sender->zone, &outgoing, sizeof(SkillUseOutgoing), &sender->pos, 60, sender->id, target->id, 5);
    }

    void set_ability(CUser* user, int typeEffect, SkillAbilityType abilityType, int abilityValue)
    {
        switch (abilityType)
        {
        // itemId: 101112, 101113
        // skillId: 432
        case SkillAbilityType::IncreaseQuestExpRate:
            user->increaseQuestExpRate += abilityValue;
            break;
        // skillId: 375
        case SkillAbilityType::AbilityStrToMaxHealth:
        {
            auto value = user->abilityStrength * abilityValue;
            user->maxHealth += value;

            CUser::SendMaxHP(user);
            CUser::SetAttack(user);
            break;
        }
        // skillId: 376
        case SkillAbilityType::AbilityRecToMaxHealth:
        {
            auto value = user->abilityReaction * abilityValue;
            user->maxHealth += value;

            CUser::SendMaxHP(user);
            CUser::SetAttack(user);
            break;
        }
        // skillId: 377
        case SkillAbilityType::AbilityIntToMaxHealth:
        {
            auto value = user->abilityIntelligence * abilityValue;
            user->maxHealth += value;

            CUser::SendMaxHP(user);
            CUser::SetAttack(user);
            break;
        }
        // skillId: 378
        case SkillAbilityType::AbilityWisToMaxHealth:
        {
            auto value = user->abilityWisdom * abilityValue;
            user->maxHealth += value;

            CUser::SendMaxHP(user);
            CUser::SetAttack(user);
            break;
        }
        // skillId: 379
        case SkillAbilityType::AbilityDexToMaxHealth:
        {
            auto value = user->abilityDexterity * abilityValue;
            user->maxHealth += value;

            CUser::SendMaxHP(user);
            CUser::SetAttack(user);
            break;
        }
        // skillId: 380
        case SkillAbilityType::AbilityLucToMaxHealth:
        {
            auto value = user->abilityLuck * abilityValue;
            user->maxHealth += value;

            CUser::SendMaxHP(user);
            CUser::SetAttack(user);
            break;
        }
        default:
            break;
        }
    }
}

unsigned u0x45CD11 = 0x45CD11;
void __declspec(naked) naked_0x45CCE3()
{
    __asm
    {
        pushad

        lea eax,[esp+0x3C]

        push eax // packet
        push esi // skillInfo
        push edi // target
        push ebp // sender
        call user_apply_skill::send_view
        add esp,0x10

        popad

        jmp u0x45CD11
    }
}

unsigned u0x493BCF = 0x493BCF;
unsigned u0x493C3F = 0x493C3F;
void __declspec(naked) naked_0x493BC6()
{
    __asm
    {
        // abilityType1
        movzx edx,byte ptr[ebp+0x7C]
        cmp edx,0x46
        je _0x493C3F

        // original
        mov edx,[ebp+0x5C]
        imul edx,edx,0x3E8
        jmp u0x493BCF

        _0x493C3F:
        jmp u0x493C3F
    }
}

unsigned u0x49DB20 = 0x49DB20;
unsigned u0x428ADA = 0x428ADA;
void __declspec(naked) naked_0x428AD5()
{
    __asm
    {
        // original
        call u0x49DB20

        pushad

        lea edx,[esi-0xD0]
        push edx // user
        call user_apply_skill::ability_70_update
        add esp,0x4

        popad

        jmp u0x428ADA
    }
}

unsigned u0x498623 = 0x498623;
void __declspec(naked) naked_0x49861D()
{
    __asm
    {
        pushad

        push esi // user
        call user_apply_skill::ability_70_remove
        add esp,0x4

        popad

        // original
        mov eax,[esi+0x1C4]
        jmp u0x498623
    }
}

void __declspec(naked) naked_0x4959A4()
{
    __asm
    {
        pushad

        inc edx

        push eax // abilityValue
        push edx // abilityType
        push ecx // typeEffect
        push esi // user
        call user_apply_skill::set_ability
        add esp,0x10

        popad

        // original
        pop esi
        mov esp,ebp
        pop ebp
        retn 0x4
    }
}

void hook::user_apply_skill()
{
    // CUser::SkillAttackRange
    util::detour((void*)0x45CCE3, naked_0x45CCE3, 6);
    // CUser::AddApplySkill
    util::detour((void*)0x493BC6, naked_0x493BC6, 9);
    // CZone::UpdateApplySkill
    util::detour((void*)0x428AD5, naked_0x428AD5, 5);
    // CUser::ClearApplySkillByDeath
    util::detour((void*)0x49861D, naked_0x49861D, 6);
    // CUser::SetSkillAbility (default case)
    util::detour((void*)0x4959A4, naked_0x4959A4, 7);
}
