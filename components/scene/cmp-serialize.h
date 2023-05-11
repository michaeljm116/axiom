/**
 * @file cmp-serialize.h
 * @author Mike Murrell (mikestoleyobike@aim.com)
 * @brief Serialize Component
 * @version 0.1
 * @date 2023-05-11
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once
#include "cmp-transform.h"

#include <flecs.h>
namespace axiom
{
    namespace serialize
    {
            enum ComponentFlag {
            COMPONENT_NODE = 0x01,
            COMPONENT_TRANSFORM = 0x02,
            COMPONENT_MATERIAL = 0x04,
            COMPONENT_LIGHT = 0x08,
            COMPONENT_CAMERA = 0x10,
            COMPONENT_MODEL = 0x20,
            COMPONENT_MESH = 0x40,
            COMPONENT_BOX = 0x80,
            COMPONENT_SPHERE = 0x0100,
            COMPONENT_PLANE = 0x0200,
            COMPONENT_AABB = 0x0400,
            COMPONENT_CYLINDER = 0x0800,
            COMPONENT_SKINNED = 0x1000,
            COMPONENT_RIGIDBODY = 0x2000,
            COMPONENT_CCONTROLLER = 0x4000,
            COMPONENT_PRIMITIVE = 0x8000,
            COMPONENT_COLIDER = 0x10000,
            COMPONENT_IMPULSE = 0X20000,
            COMPONENT_GUI = 0X40000,
            COMPONENT_BUTTON = 0x80000,
            COMPONENT_JOINT = 0x100000,
            COMPONENT_HEADNODE = 0x20000
        }; 
/*
        int64_t To_ComponentFlag(flecs::entity e){
            int64_t flag = 0;
            if(e.get<Cmp_Transform>() != nullptr) flag |= COMPONENT_TRANSFORM;
            return flag;
        };*/

    }
    struct Cmp_Serialize{
        int64_t engine_flags;
        int64_t game_flags;
    };
}