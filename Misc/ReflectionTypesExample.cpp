#include <iostream>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <typeinfo>

using UUID = boost::uuids::uuid;

UUID generateUUID() {
    static boost::uuids::random_generator generator;
    return generator();
}

struct MemberVariableInfo {
    std::string name;
    std::string type;
    size_t offset;
};

struct TypeInfo {
    UUID id;
    std::string name;
    std::function<void*()> createInstance;
    std::vector<MemberVariableInfo> memberVariables;
};

class TypeRegistry {
public:
    static TypeRegistry& instance() {
        static TypeRegistry instance;
        return instance;
    }

    void registerType(const TypeInfo& info) {
        registry[info.id] = info;
    }

    const TypeInfo* getTypeInfo(const UUID& id) const {
        auto it = registry.find(id);
        return it != registry.end() ? &it->second : nullptr;
    }

    void registerMemberVariable(const UUID& typeId, const MemberVariableInfo& memberInfo) {
        auto it = registry.find(typeId);
        if (it != registry.end()) {
            it->second.memberVariables.push_back(memberInfo);
        }
    }

private:
    std::unordered_map<UUID, TypeInfo> registry;
};

class Reflectable {
public:
    virtual ~Reflectable() = default;
    virtual const TypeInfo& getTypeInfo() const = 0;
};

#define REGISTER_MEMBER(Type, Member) \
    { \
        TypeRegistry::instance().registerMemberVariable( \
            Type::typeId, \
            MemberVariableInfo{#Member, typeid(decltype(Type::Member)).name(), offsetof(Type, Member)} \
        ); \
    }

#define DECLARE_REFLECTABLE(Type) \
public: \
    static UUID typeId; \
    const TypeInfo& getTypeInfo() const override { \
        static const TypeInfo* info = TypeRegistry::instance().getTypeInfo(typeId); \
        return *info; \
    } \
    static void registerMembers(); \
private: \
    static bool registered;

#define IMPLEMENT_REFLECTABLE(Type) \
    UUID Type::typeId; \
    bool Type::registered = (Type::registerMembers(), true);

#define REGISTER_TYPE(Type) \
    namespace { \
        struct Type##Registration { \
            Type##Registration() { \
                TypeInfo info; \
                info.id = generateUUID(); \
                info.name = #Type; \
                info.createInstance = []() -> void* { return new Type(); }; \
                Type::typeId = info.id; \
                TypeRegistry::instance().registerType(info); \
                Type::registerMembers(); \
            } \
        }; \
        static Type##Registration global_##Type##Registration; \
    }

class MyComponent : public Reflectable {
    DECLARE_REFLECTABLE(MyComponent)
public:
    int a;
    float b;
    std::string c;

    MyComponent() : a(0), b(0.0f), c("") {}
};

IMPLEMENT_REFLECTABLE(MyComponent)

void MyComponent::registerMembers() {
    REGISTER_MEMBER(MyComponent, a);
    REGISTER_MEMBER(MyComponent, b);
    REGISTER_MEMBER(MyComponent, c);
}

REGISTER_TYPE(MyComponent)

int main() {
    MyComponent comp;
    const TypeInfo& info = comp.getTypeInfo();
    std::cout << "Component Type: " << info.name << std::endl;
    std::cout << "UUID: " << info.id << std::endl;

    for (const auto& member : info.memberVariables) {
        std::cout << "Member: " << member.name << ", Type: " << member.type << ", Offset: " << member.offset << std::endl;
    }

    return 0;
}
