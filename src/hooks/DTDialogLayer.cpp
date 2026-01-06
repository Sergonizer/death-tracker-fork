#include "DTDialogLayer.hpp"

void DTDialogLayer::displayDialogObject(DialogObject* dialogue){
    DialogLayer::displayDialogObject(dialogue);

    if (m_fields->callback != NULL)
        m_fields->callback(dialogue);

    std::string cacDialogSpr = "";
    std::tuple<std::string, int, float> cacDialogSprinfo;

    for (const auto& existingIcons : m_fields->icons)
    {
        if (std::get<1>(existingIcons) == dialogue->m_characterFrame){
            cacDialogSpr = std::get<0>(existingIcons);
            cacDialogSprinfo = existingIcons;
            break;
        }
    }

    if (cacDialogSpr != "") {
        m_characterSprite->setVisible(false);

        auto icon = m_mainLayer->getChildByID("cac_portrait"_spr);
        if (icon != nullptr) icon->removeMeAndCleanup();

        auto newIcon = CCSprite::createWithSpriteFrameName(cacDialogSpr.c_str());
        if (newIcon == nullptr) newIcon = CCSprite::create(cacDialogSpr.c_str());
        if (newIcon == nullptr) return;
        newIcon->setID("cac_portrait"_spr);
        newIcon->setPosition(m_characterSprite->getPosition());
        newIcon->setZOrder(4);
        newIcon->setScale(std::get<2>(cacDialogSprinfo));
        m_mainLayer->addChild(newIcon);
    }
    else{
        auto icon = m_mainLayer->getChildByID("cac_portrait"_spr);
        if (icon != nullptr) icon->setVisible(false);
    }
}

void DTDialogLayer::useTaggedSprites(const std::vector<std::tuple<std::string, int, float>>& icons){
    m_fields->icons = icons;
}

DialogLayer* DTDialogLayer::createWithTaggedSprites(CCArray* objects, int bg, const std::vector<std::tuple<std::string, int, float>>& icons){
    objects->insertObject(DialogObject::create("", "", 0, 1, true, {0,0,0}), 0);
    auto ret = DialogLayer::createWithObjects(objects, bg);
    // static_cast<CACDialogLayer*>(ret)->useTaggedSprites(icons);
    // ret->displayNextObject();

    return ret;
}

void DTDialogLayer::setProgressCallback(const std::function<void(DialogObject*)>& callback){
    m_fields->callback = callback;
}

bool DTDialogLayer::init(DialogObject* object, cocos2d::CCArray* objects, int background){
    if (object != nullptr){
        m_fields->firstDialogue = object;
    }
    else if (objects != nullptr && objects->count() != 0){
        m_fields->firstDialogue = static_cast<DialogObject*>(objects->firstObject());
    }

    if (!DialogLayer::init(object, objects, background)) return false;

    return true;
}

void DTDialogLayer::addToMainScene(){
    DialogLayer::addToMainScene();
    if (m_fields->callback != NULL && m_fields->firstDialogue != nullptr){
        m_fields->callback(m_fields->firstDialogue);
    }
}