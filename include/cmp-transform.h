#include <flecs.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

struct Cmp_Transform
{
    glm::vec4 pos;
    glm::quat rot;
    glm::vec4 sca;
    
    glm::vec3 euler_rot;
    glm::mat4 trans_rot;
    glm::mat4 world;
};
