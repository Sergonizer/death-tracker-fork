#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

#include <Geode/modify/DialogLayer.hpp>

class $modify(DTDialogLayer, DialogLayer) {
    void useTaggedSprites(const std::vector<std::tuple<std::string, int, float>>& icons);

    void displayDialogObject(DialogObject* dialogue);

    static DialogLayer* createWithTaggedSprites(CCArray* objects, int bg, const std::vector<std::tuple<std::string, int, float>>& icons);

    void setProgressCallback(const std::function<void(DialogObject*)>& callback);

    bool init(DialogObject* object, cocos2d::CCArray* objects, int background);
    void addToMainScene();

    struct Fields{
        std::vector<std::tuple<std::string, int, float>> icons;
        std::function<void(DialogObject*)> callback = NULL;
        DialogObject* firstDialogue = nullptr;
        bool doCallCreationCallback = false;
    };
};