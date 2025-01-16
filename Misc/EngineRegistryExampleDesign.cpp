// Base interface for all systems
template<class T>
class IRZSystem {
public:
    virtual ~IRZSystem() = default;
    virtual void StartUp() = 0;
    virtual void ShutDown() = 0;
    virtual void Update() = 0;

    // Singleton pattern
    static T& Instance() {
        static T instance;
        return instance;
    }

protected:
    IRZSystem() = default;
};

//---------------------------------------------------------------------

class SystemRegistry {
public:
    static SystemRegistry& Instance() {
        static SystemRegistry instance;
        return instance;
    }

    template<class T>
    void RegisterSystem() {
        const std::string typeName = typeid(T).name();
        if (systems.find(typeName) == systems.end()) {
            systems[typeName] = &T::Instance();
        }
    }

    IRZSystem<void>* GetSystem(const std::string& typeName) {
        auto it = systems.find(typeName);
        return it != systems.end() ? it->second : nullptr;
    }

    void StartUpAll() {
        for (auto& pair : systems) {
            pair.second->StartUp();
        }
    }

    void ShutDownAll() {
        for (auto& pair : systems) {
            pair.second->ShutDown();
        }
    }

    void UpdateAll() {
        for (auto& pair : systems) {
            pair.second->Update();
        }
    }

private:
    std::unordered_map<std::string, IRZSystem<void>*> systems;
};

//---------------------------------------------------------------------

#define REGISTER_ENGINE_SYSTEM(SystemType) \
    namespace { \
        struct SystemType##Registration { \
            SystemType##Registration() { \
                SystemRegistry::Instance().RegisterSystem<SystemType>(); \
            } \
        }; \
        static SystemType##Registration global_##SystemType##Registration; \
    }