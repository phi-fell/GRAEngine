#ifndef GRAE_ENGINE_RESOURCES_H
#define GRAE_ENGINE_RESOURCES_H

#include "system/log.h"

#include <string>
#include <unordered_map>
#include <typeinfo>
#include <typeindex>
#include <vector>

namespace GRAE {
class Resources;
namespace PRIVATE {
template<typename T>
std::string getTypename() {
    return typeid(T).name();
}

class ResourceHandler {
public:
    virtual ~ResourceHandler() = 0;
};

template<typename T>
class Handler : public ResourceHandler {
private:
    std::unordered_map<std::string, T *> resources;
    std::string directory;
public:
    explicit Handler(std::string dir) : directory(dir) {}

    ~Handler() override {
        log->debug << "Freeing all <" << PRIVATE::getTypename<T>() << ">";
        for (auto pair : resources) {
            delete pair.second;
            log->verbose << pair.first << " unloaded";
        }
    }

    virtual T *&operator[](std::string id) { return resources[id]; }

    virtual bool resourceExists(std::string id) {
        return (bool) resources.count(id);
    }

    virtual std::string getDir() {
        return directory;
    }
};

class DefaultResource {
public:
    virtual ~DefaultResource() = 0;
};

template<typename T>
class DefaultResourceImpl : public DefaultResource {
public:
    T *resource;

    explicit DefaultResourceImpl(Resources *res) : resource(new T(res)) {}
    ~DefaultResourceImpl(){
        delete resource;
        log->verbose << "Default<" << getTypename<T>() << "> unloaded";
    }
};
}
class Resources {
private:
    std::unordered_map<std::type_index, PRIVATE::ResourceHandler *> handlers;
    std::unordered_map<std::type_index, PRIVATE::DefaultResource *> defaults;
    std::string rootDir;
private:
    template<typename T>
    T *const &getDefault() {
        if (!defaults.count(std::type_index(typeid(T)))) {
            log->debug << "Loading default Resource<" << PRIVATE::getTypename<T>() << ">";
            defaults[std::type_index(typeid(T))] = new PRIVATE::DefaultResourceImpl<T>(this);
        }
        return ((PRIVATE::DefaultResourceImpl<T> *) defaults[std::type_index(typeid(T))])->resource;
    }

    template<typename T>
    T *const &getResource(std::string id) {
        log->verbose << "Resource<" << PRIVATE::getTypename<T>() << "> requested: \"" << id << "\"";
        if (!handlers.count(std::type_index(typeid(T)))) {
            log->err << "Resource type not initialized!";
            log->verbose << PRIVATE::getTypename<T>();
            return getDefault<T>();
        }
        if (!((PRIVATE::Handler<T> *) (handlers[std::type_index(typeid(T))]))->resourceExists(id)) {
            log->info << "Loading Resource<" << PRIVATE::getTypename<T>() << ">: \"" << id << "\"";
            bool success = false;
            std::string reason = "reason unknown";
            T *resource = new T(id, this, success, reason);
            if (!success) {
                log->err << "Failed to load <" << PRIVATE::getTypename<T>() << ">: " << id << " - " << reason;
                delete resource;
                return getDefault<T>();
            } else {
                ((*(PRIVATE::Handler<T> *) (handlers[std::type_index(typeid(T))])))[id] = resource;
                log->debug << "Successfully Loaded Resource<" << PRIVATE::getTypename<T>() << ">: \"" << id << "\"";
            }
        }
        return ((*(PRIVATE::Handler<T> *) (handlers[std::type_index(typeid(T))])))[id];
    }

public:
    explicit Resources(std::string root) : rootDir(root) {
        log->info << "Initialized Resources at \"" << root << "\"";
    }

    ~Resources() {
        log->info << "Unloading Resources...";
        for (auto pair : handlers) {
            delete pair.second;
        }
        log->info << "Freeing Defaults";
        for (auto pair : defaults) {
            delete pair.second;
        }
        log->info << "All Resources Freed";
    }

    template<typename T>
    void initResourceType(std::string dir) {
        log->debug << "Initializing Resource Type <" << PRIVATE::getTypename<T>() << ">";
        handlers[std::type_index(typeid(T))] = new PRIVATE::Handler<T>(dir);
    }

    template<typename T>
    T *const &get(std::string id = "") {
        if (id.size() > 0) {
            return getResource<T>((rootDir.length() > 0 ? rootDir + "/" : "") +
                                  (((*(PRIVATE::Handler<T> *) (handlers[std::type_index(
                                          typeid(T))]))).getDir().length() > 0
                                   ? ((*(PRIVATE::Handler<T> *) (handlers[std::type_index(typeid(T))]))).getDir() + "/"
                                   : "") + id);
        } else {
            return getDefault<T>();
        }
    }


    template<typename T>
    T *const &getFromRoot(std::string id) {
        if (id.size() > 0) {
            return getResource<T>(id);
        } else {
            return getDefault<T>();
        }
    }
};
}

#endif //GRAE_ENGINE_RESOURCES_H
