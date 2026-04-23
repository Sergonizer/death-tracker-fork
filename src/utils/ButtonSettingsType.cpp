#include "ButtonSettingsType.hpp"

Result<std::shared_ptr<SettingV3>> ButtonSettingDT::parse(std::string const& key, std::string const& modID, matjson::Value const& json) {
    auto res = std::make_shared<ButtonSettingDT>();
    auto root = checkJson(json, "ButtonSettingV3");

    res->init(key, modID, root);
    res->parseNameAndDescription(root);
    res->parseEnableIf(root);
    
    root.checkUnknownKeys();
    return root.ok(std::static_pointer_cast<SettingV3>(res));
}

bool ButtonSettingDT::load(matjson::Value const& json) {
    return true;
}
bool ButtonSettingDT::save(matjson::Value& json) const {
    return true;
}

bool ButtonSettingDT::isDefaultValue() const {
    return true;
}
void ButtonSettingDT::reset() {}

SettingNodeV3* ButtonSettingDT::createNode(float width) {
    return ButtonSettingNodeDT::create(
        std::static_pointer_cast<ButtonSettingDT>(shared_from_this()),
        width
    );
}


bool ButtonSettingNodeDT::init(std::shared_ptr<ButtonSettingDT> setting, float width) {
    if (!SettingNodeV3::init(setting, width))
        return false;
        
    m_buttonSprite = ButtonSprite::create("Convert", "goldFont.fnt", "GJ_button_01.png", .8f);
    m_buttonSprite->setScale(.5f);
    m_button = CCMenuItemSpriteExtra::create(
        m_buttonSprite, this, menu_selector(ButtonSettingNodeDT::onButton)
    );
    this->getButtonMenu()->addChildAtPosition(m_button, Anchor::Center);
    this->getButtonMenu()->setContentWidth(60);
    this->getButtonMenu()->updateLayout();

    this->updateState(nullptr);
    
    return true;
}

void ButtonSettingNodeDT::updateState(CCNode* invoker) {
    SettingNodeV3::updateState(invoker);

    auto shouldEnable = this->getSetting()->shouldEnable();
    m_button->setEnabled(shouldEnable);
    m_buttonSprite->setCascadeColorEnabled(true);
    m_buttonSprite->setCascadeOpacityEnabled(true);
    m_buttonSprite->setOpacity(shouldEnable ? 255 : 155);
    m_buttonSprite->setColor(shouldEnable ? ccWHITE : ccGRAY);
}
void ButtonSettingNodeDT::onButton(CCObject*) {
    FileConversionLayer::create(false)->show();
}

void ButtonSettingNodeDT::onCommit() {}
void ButtonSettingNodeDT::onResetToDefault() {}

ButtonSettingNodeDT* ButtonSettingNodeDT::create(std::shared_ptr<ButtonSettingDT> setting, float width) {
    auto ret = new ButtonSettingNodeDT();
    if (ret->init(setting, width)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ButtonSettingNodeDT::hasUncommittedChanges() const {
    return false;
}
bool ButtonSettingNodeDT::hasNonDefaultValue() const {
    return false;
}

std::shared_ptr<ButtonSettingDT> ButtonSettingNodeDT::getSetting() const {
    return std::static_pointer_cast<ButtonSettingDT>(SettingNodeV3::getSetting());
}