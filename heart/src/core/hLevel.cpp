/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "core/hLevel.h"
#include "base/hUUID.h"
#include "base/hStringUtil.h"
#include "components/hEntity.h"
#include "components/hEntityFactory.h"
#include "debug/hDebugMenuManager.h"
#include "imgui.h"

namespace Heart {
hRegisterObjectType(LevelDefinition, Heart::hLevel, Heart::proto::LevelDefinition);

hBool hLevel::serialiseObject(Heart::proto::LevelDefinition* obj) const {
    return hTrue;
}

hBool hLevel::deserialiseObject(Heart::proto::LevelDefinition* obj) {
    std::vector<hComponentDefinition> components;
    levelName = obj->levelname();
    levelEntities.reserve(obj->entities_size());
    for (hInt i=0, n=obj->entities_size(); i<n; ++i) {
        auto& entity = obj->entities(i);
        hUuid_t guid = hUUID::fromString(entity.objectguid().c_str(), entity.objectguid().length());
        hSize_t totalComponents = entity.components_size();
#if HEART_DEBUG_INFO
        totalComponents += entity.debugcomponents_size();
#endif
        components.clear();
        components.reserve(totalComponents);
        for (hInt ci = 0, cn = entity.components_size(); ci<cn; ++ci) {
            hComponentDefinition comp_def;
            comp_def.typeDefintion = hObjectFactory::getObjectDefinitionFromSerialiserName(entity.components(ci).type_name().c_str());
            if (comp_def.typeDefintion) {
                comp_def.marshall = comp_def.typeDefintion->constructMarshall_();
                comp_def.marshall->ParseFromString(entity.components(ci).messagedata());
                components.push_back(std::move(comp_def));
            }
        }
#if HEART_DEBUG_INFO
        for (hInt ci = 0, cn = entity.debugcomponents_size(); ci < cn; ++ci) {
            hComponentDefinition comp_def;
            comp_def.typeDefintion = hObjectFactory::getObjectDefinitionFromSerialiserName(entity.debugcomponents(ci).type_name().c_str());
            if (comp_def.typeDefintion) {
                comp_def.marshall = comp_def.typeDefintion->constructMarshall_();
                comp_def.marshall->ParseFromString(entity.components(ci).messagedata());
                components.push_back(std::move(comp_def));
            }
        }
#endif
        auto* lvl_entity = hEntityFactory::createEntity(guid, components.data(), components.size());
        lvl_entity->transient = entity.has_transient() ? entity.transient() : false;
        levelEntities.push_back(lvl_entity);
    }
#if HEART_DEBUG_INFO
    hSize_t len = levelName.size() + 7;
    hChar* debugLevelName = (hChar*)hAlloca(len);
    hStrCopy(debugLevelName, len, "Level:");
    hStrCat(debugLevelName, len, levelName.c_str());
    hDebugMenuManager::registerMenu(debugLevelName, [&]() {
        hChar guid_str[64], tmp_buf[64];
        hUint obj_num = 0;
        hSize_t len = levelName.size() + 7;
        hChar* debugLevelName = (hChar*)hAlloca(len);
        hStrCopy(debugLevelName, len, "Level:");
        hStrCat(debugLevelName, len, levelName.c_str());
        ImGui::Begin(debugLevelName, nullptr, ImGuiWindowFlags_ShowBorders);
        for (const auto& i : levelEntities) {
            hUUID::toString(i->entityId, guid_str, sizeof(guid_str));
            if (ImGui::TreeNode(guid_str)) {
                ImGui::Text("Transient:%s", i->transient ? "yes" : "no");
                hStrPrintf(tmp_buf, sizeof(tmp_buf), "Component %d", obj_num);
                if (ImGui::TreeNode(tmp_buf)) {
                    for (const auto& ci : i->entityComponents) {
                        ImGui::Text("Type:%s", ci.typeDef->objectName_.c_str()); ImGui::SameLine();
                        ImGui::Text("Address:0x%p", ci.ptr);
                    }
                    ImGui::TreePop();
                }
                ImGui::TreePop();
            }
            ++obj_num;
        }
        ImGui::End();
    });
#endif
    return hTrue;
}

hBool hLevel::linkObject() {
    hBool linked = hTrue;
    for (const auto& i : levelEntities) {
        for (const auto& c : i->entityComponents) {
            linked &= c.typeDef->link_(c.ptr);
        }
    }
    return linked;
}

hLevel::~hLevel() {
#if HEART_DEBUG_INFO
    hSize_t len = levelName.size() + 7;
    hChar* debugLevelName = (hChar*)hAlloca(len);
    hStrCopy(debugLevelName, len, "Level:");
    hStrCat(debugLevelName, len, levelName.c_str());
    hDebugMenuManager::unregisterMenu(debugLevelName);
#endif
}

}