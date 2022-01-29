#pragma once
#include "cfx_model.hpp"
#include <memory>
#include <unordered_map>
#include <glm/gtc/matrix_transform.hpp>

namespace cfx{

    struct TransformComponent {
        glm::vec3 translation{};
        glm::vec3 scale{1.f,1.f,1.f};
        glm::vec3 rotation{};
        glm::mat4 mat4();
        glm::mat3 normalMatrix();
        
    };

    struct PointLightComponent{
        float lightIntensity = 1.0f;

    };


    class CFXGameObject{
        public:
        using id_t = unsigned int;
        using Map = std::unordered_map<id_t,CFXGameObject>;
        static CFXGameObject createGameObject(){
            static id_t currentId = 0;
            return CFXGameObject{currentId++};

        }
        static CFXGameObject makePointLight(float intensity = 10.0f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.0f));
        CFXGameObject(const CFXGameObject &) = delete;
        CFXGameObject &operator=(const CFXGameObject &) = delete;
        CFXGameObject(CFXGameObject &&) = default;
        CFXGameObject &operator=(CFXGameObject &&) = default;

        id_t getId() { return id;}
        
        glm::vec3 color{};
        TransformComponent transformComponent{};

        std::shared_ptr<CFXModel> model{};
        std::unique_ptr<PointLightComponent> pointLight = nullptr;

        private:
        id_t id;
        CFXGameObject(id_t objId): id{objId} {

        }
        

    };
}