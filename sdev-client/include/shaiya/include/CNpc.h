#pragma once
#include <array>
#include <shaiya/include/npc/NpcType.h>
#include "include/shaiya/common.h"

namespace shaiya
{
    struct CStaticText;

    using NpcName = std::array<char, 50>;

    #pragma pack(push, 1)
    // 0049AB60 ctor
    struct CNpc
    {
        PAD(4);
        NpcName name;           //0x04
        PAD(2);
        D3DVECTOR pos;          //0x38
        D3DVECTOR dir;          //0x44
        D3DVECTOR up;           //0x50
        ULONG id;               //0x5C
        NpcType8 type;          //0x60
        PAD(1);
        UINT16 typeId;          //0x62
        PAD(28);
        BOOL isMoving;          //0x80
        PAD(8);
        D3DVECTOR movePos;      //0x8C
        D3DVECTOR moveDir;      //0x98
        UINT32 model;           //0xA4
        PAD(100);
        CStaticText* typeText;  //0x10C
        long typePointX;        //0x110
        CStaticText* nameText;  //0x114
        long namePointX;        //0x118
        PAD(16);
        // 0x12C
    };
    #pragma pack(pop)
}
