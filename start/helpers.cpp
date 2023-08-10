#include "helpers.h"

glm::vec3 maxV (const glm::vec3& a, const glm::vec3& b){
    glm::vec3 ret;
    ret.x = std::max(a.x, b.x);
    ret.y = std::max(a.y, b.y);
    ret.z = std::max(a.z, b.z);
    return ret;
};

glm::vec3 minV (const glm::vec3& a, const glm::vec3& b){
    glm::vec3 ret;
    ret.x = std::min(a.x, b.x);
    ret.y = std::min(a.y, b.y);
    ret.z = std::min(a.z, b.z);
    return ret;
};  