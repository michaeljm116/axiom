#pragma once
#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif

#ifndef GLM_FORCE_SSE2 
#define GLM_FORCE_SSE2 
#endif
#include <flecs.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/simd/common.h>
#include <vector>
#include <queue>
#include "cmp-resource.h"

namespace axiom{

    struct sqt{
        glm::quat rot = glm::quat();
        glm::vec4 pos = glm::vec4(0);
        glm::vec4 sca = glm::vec4(1);
    }; //48 bytes
    inline bool operator==(const sqt& a, const sqt& b) {
		uint_fast8_t p = (a.pos == b.pos);
		uint_fast8_t r = (a.rot == b.rot) << 1;
		uint_fast8_t s = (a.sca == b.sca) << 2;
		uint_fast8_t ret = p | r | s;
		return (ret == 7);
	};

    struct Cmp_Transform{
        glm::mat4 world;
        glm::mat4 trm;
        sqt local;
        sqt global;
        glm::vec3 euler_rot;

        Cmp_Transform(){}
        Cmp_Transform(glm::vec3 center, glm::vec3 extents){
        }
        Cmp_Transform(glm::vec3 pos, glm::vec3 rot, glm::vec3 sca) : euler_rot(rot){
            glm::mat4 rot_m;
            rot_m = glm::rotate(rot_m, glm::radians(rot.x), glm::vec3(1.0f, 0.f, 0.f));
            rot_m = glm::rotate(rot_m, glm::radians(rot.y), glm::vec3(0.0f, 1.f, 0.f));
            rot_m = glm::rotate(rot_m, glm::radians(rot.z), glm::vec3(0.0f, 0.f, 1.f));
            local.rot = glm::toQuat(rot_m);
            local.pos = glm::vec4(pos, 0.f);
            local.sca = glm::vec4(sca, 0.f);

            trm = rot_m;
            trm[3] = glm::vec4(pos, 1.f);
            world = glm::scale(trm, sca);

        }
    };
}
