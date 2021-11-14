#pragma once
#include "cfx_model.hpp"
#include <memory>
#include <glm/gtc/matrix_transform.hpp>

namespace cfx{

    struct TransformComponent {
        glm::vec3 translation{};
        glm::vec3 scale{1.f,1.f,1.f};
        glm::vec3 rotation{};
        glm::mat4 mat4();
        glm::mat3 normalMatrix();
        
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
        TransformComponent transformComponent{};

        private:
        id_t id;
        CFXGameObject(id_t objId): id{objId} {

        }
        

    };
}