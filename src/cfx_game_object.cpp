#include "cfx_game_object.hpp"

namespace cfx {
    glm::mat4 TransformComponent::mat4(){
            auto transform = glm::translate(glm::mat4{1.f},translation);
            transform = glm::rotate(transform,rotation.y,{0.f,1.f,0.f});
            transform = glm::rotate(transform,rotation.x,{1.f,0.f,0.f});
            transform = glm::rotate(transform,rotation.z,{0.f,0.f,1.f});
            transform = glm:: scale(transform,scale);
            return transform;


        }

    glm::mat3 TransformComponent::normalMatrix(){

        const float c3 = glm::cos(rotation.z);
        const float s3 = glm::sin(rotation.z);
        const float c2 = glm::cos(rotation.x);
        const float s2 = glm::cos(rotation.x);
        const float c1 = glm::cos(rotation.y);
        const float s1 = glm::cos(rotation.y);

        const glm::vec3 invScale = 1.0f / scale;
        return glm::mat3 {
            {
                invScale.x * (c1 * c3 + s1 * s2 * s3), 
                invScale.x * (c2 * c3), 
                invScale.x * (c1 * s2 * s3 - c3 * s1),
            },
            {
                invScale.y * (c3 * s1 * s2 -  c1 * s3), 
                invScale.y * (c2 * c3), 
                invScale.y * (c1 * c3 * s2 + s1 * s3),
            },
            {
                invScale.z * (c2 * s1), 
                invScale.z * (-s2), 
                invScale.z * (c1 * c2),
            }

        };

      


    }

}