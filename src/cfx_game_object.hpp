#pragma once
#include "cfx_model.hpp"
#include <memory>

namespace cfx{
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

        private:
        id_t id;
        CFXGameObject(id_t objId): id{objId} {

        }
        

    };
}