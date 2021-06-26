#pragma once
#include "cfx_model.hpp"
#include <memory>

namespace cfx{

    struct Transform2dComponent {
        glm::vec2 translation{};
        glm::vec2 scale{1.f,1.f};
        float rotation;
        glm::mat2 mat2() {
            const float s = glm::sin(rotation);
            const float c = glm::cos(rotation);
            glm::mat2 rotationMatrix{{c,s},{-s,-c}};
            glm::mat2 scaleMat{{scale.x,.0f},{.0f,scale.y}};
            return rotationMatrix * scaleMat;
            }
    };


    class CFXGameObject{
        public:
        using id_t = unsigned int;
        static CFXGameObject createGameObject(){
            static id_t currentId = 0;
            return CFXGameObject{currentId++};

        }
        CFXGameObject(const CFXGameObject &) = delete;
        CFXGameObject &operator=(const CFXGameObject &) = delete;
        CFXGameObject(CFXGameObject &&) = default;
        CFXGameObject &operator=(CFXGameObject &&) = default;

        id_t getId() { return id;}
        std::shared_ptr<CFXModel> model{};
        glm::vec3 color{};
        Transform2dComponent transform2d;

        private:
        id_t id;
        CFXGameObject(id_t objId): id{objId} {

        }
        

    };
}