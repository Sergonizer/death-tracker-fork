#include "GraphCell.hpp"
#include <managers/StatsManager.hpp>

GraphCell* GraphCell::create(float width, const DTGraphInfo& graphInfo){
    auto ret = new GraphCell();
    if (ret && ret->init(width, graphInfo)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool GraphCell::init(float width, const DTGraphInfo& graphInfo){
    if (!CCMenu::init()) return false;

    this->setContentSize({width, 45});

    this->graphInfo = graphInfo;

    auto bg = CCScale9Sprite::create("square01_001.png");
    bg->setAnchorPoint({0, 0});
    bg->setScale(.2f);
    bg->setContentSize((this->getContentSize() - ccp(0, 2.5f)) / bg->getScale());
    this->addChild(bg);

    label = SimpleTextArea::create(graphInfo.name, "bigFont.fnt");
    label->setScale(.25f);
    label->setAnchorPoint({0, 1});
    label->setPosition(ccp(0 + 3, this->getContentHeight() - 5));
    label->setWidth(width - 10);
    label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
    this->addChild(label);

    outerColor = CCSprite::create("circle.png");
    outerColor->setColor({graphInfo.outlineColor.r, graphInfo.outlineColor.g, graphInfo.outlineColor.b});
    outerColor->setOpacity(graphInfo.outlineColor.a);

    innerColor = CCSprite::create("circle.png");
    innerColor->setScale(.75f);
    innerColor->setPosition(outerColor->getContentSize() / 2);
    innerColor->setColor({graphInfo.color.r, graphInfo.color.g, graphInfo.color.b});
    innerColor->setOpacity(graphInfo.color.a);
    outerColor->addChild(innerColor);

    auto disabledCircle = CCSprite::create("circle.png");
    disabledCircle->setColor({ 84, 43, 43 });

    auto disabledCircleX = CCSprite::createWithSpriteFrameName("edit_delBtnSmall_001.png");
    disabledCircleX->setPosition(disabledCircle->getContentSize() / 2);
    disabledCircleX->setScale(.4f);
    disabledCircle->addChild(disabledCircleX);

    enableToggleBtn = SimpleToggler::create(
        disabledCircle,
        outerColor,
        1.5f,
        graphInfo.isEnabled
    );
    enableToggleBtn->setPosition(label->getPosition() + ccp(
        enableToggleBtn->getContentWidth() / 2,
        -label->getScaledContentHeight() - enableToggleBtn->getContentHeight() / 2 - 2
    ));
    enableToggleBtn->setZOrder(1);
    enableToggleBtn->setID("toggle-btn");
    enableToggleBtn->setCallback([&](auto state){
        setEnabledInfo(state, false, true);
    });
    this->addChild(enableToggleBtn);

    auto bgCircle = CCSprite::create("circle.png");
    bgCircle->setColor({0,0,0});
    bgCircle->setPosition(enableToggleBtn->getPosition());
    bgCircle->setScale(1.7f);
    this->addChild(bgCircle);

    this->setZOrder(graphInfo.orderPos);

    auto arrowDownSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    arrowDownSpr->setScale(.2f);
    arrowDownSpr->setRotation(-90);
    auto arrowDown = CCMenuItemSpriteExtra::create(
        arrowDownSpr,
        this,
        menu_selector(GraphCell::onArrowDown)
    );
    arrowDown->setID("down-arrow");
    arrowDown->setPosition({
        width - arrowDown->getContentWidth() / 2 - 4,
        10
    });
    this->addChild(arrowDown);

    auto arrowUpSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    arrowUpSpr->setScale(.2f);
    arrowUpSpr->setRotation(90);
    auto arrowUp = CCMenuItemSpriteExtra::create(
        arrowUpSpr,
        this,
        menu_selector(GraphCell::onArrowUp)
    );
    arrowUp->setID("up-arrow");
    arrowUp->setPosition({
        arrowDown->getPositionX(),
        arrowDown->getPositionY() + arrowUp->getContentHeight() / 2 + arrowDown->getContentHeight() / 2 + 1
    });
    this->addChild(arrowUp);

    auto settingBtnSpr = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
    settingBtnSpr->setScale(.25f);
    auto settingBtn = CCMenuItemSpriteExtra::create(
        settingBtnSpr,
        this,
        menu_selector(GraphCell::onOptions)
    );
    settingBtn->setID("settings-btn");
    settingBtn->setScale(12 / settingBtn->getContentHeight());
    settingBtn->setPosition({enableToggleBtn->getPositionX(), enableToggleBtn->getPositionY() - enableToggleBtn->getContentHeight() / 2 - settingBtn->getContentHeight() / 2});
    this->addChild(settingBtn);

    typeSwitcher = OptionSwitcher<DTGraphType>::create(90, {
        {DTGraphType::Passrate, "Passrate"},
        {DTGraphType::Reachrate, "Reachrate"}
    });
    typeSwitcher->setID("type-switcher");
    typeSwitcher->setScale(.4f);
    typeSwitcher->setAnchorPoint({.5f, 0});
    typeSwitcher->setPositionX(width / 2 + 2);
    typeSwitcher->setPositionY(6.5f);
    typeSwitcher->setCallback([&](auto value){
        setType(value);
    });
    typeSwitcher->setValue(graphInfo.type, false);
    this->addChild(typeSwitcher);
    
    auto typeSwitcherLabel = CCLabelBMFont::create("Type:", "bigFont.fnt");
    typeSwitcherLabel->setScale(.25f);
    typeSwitcherLabel->setPosition(typeSwitcher->getPosition() + ccp(
        0,
        typeSwitcher->getScaledContentHeight() + typeSwitcherLabel->getScaledContentHeight() / 2
    ));
    typeSwitcherLabel->setID("type-label");
    this->addChild(typeSwitcherLabel); 

    return true;
}

void GraphCell::onArrowUp(CCObject*){
    if (onArrowCallback != NULL) onArrowCallback(this, true);
}
void GraphCell::onArrowDown(CCObject*){
    if (onArrowCallback != NULL) onArrowCallback(this, false);
}
void GraphCell::onOptions(CCObject*){
    if (onOptionsCallback != NULL) onOptionsCallback(this);
}

void GraphCell::setOrderPos(int pos){
    graphInfo.orderPos = pos;
    this->setZOrder(pos);
    onInfoChanged(true);
}

bool GraphCell::setName(const std::string& name){
    if (!canChangeNameTo(name, this)) return false;

    oldName = graphInfo.name;

    graphInfo.name = name;
    label->setText(name);
    onInfoChanged(true);

    return true;
}
void GraphCell::setCoverage(DTGraphCoverage coverage){
    graphInfo.coverage = coverage;
    onInfoChanged(true);
}
void GraphCell::setType(DTGraphType type){
    graphInfo.type = type;
    typeSwitcher->setValue(type, false);
    onInfoChanged(true);
}

void GraphCell::setThickness(float thickness){
    graphInfo.thickness = thickness;
    onInfoChanged(true);
}
void GraphCell::setOutlineThickness(float outlineThickness){
    graphInfo.outlineThickness = outlineThickness;
    onInfoChanged(true);
}
void GraphCell::setPointSize(float pointSize){
    graphInfo.pointScale = pointSize;
    onInfoChanged(true);
}
void GraphCell::setColor(ccColor4B color){
    graphInfo.color = color;
    innerColor->setColor({graphInfo.color.r, graphInfo.color.g, graphInfo.color.b});
    innerColor->setOpacity(graphInfo.color.a);
    onInfoChanged(true);
}
void GraphCell::setOutlineColor(ccColor4B color){
    graphInfo.outlineColor = color;
    outerColor->setColor({graphInfo.outlineColor.r, graphInfo.outlineColor.g, graphInfo.outlineColor.b});
    outerColor->setOpacity(graphInfo.outlineColor.a);
    onInfoChanged(true);
}
void GraphCell::setPointColor(ccColor4B color){
    graphInfo.pointColor = color;
    onInfoChanged(true);
}

void GraphCell::onInfoChanged(bool updateGraph){
    if (updateGraph && onInfoChangedCallback != NULL)
        onInfoChangedCallback(this);
}

void GraphCell::setEnabledInfo(bool b, bool changeToggler, bool callback){
    this->graphInfo.isEnabled = b;
    if (changeToggler)
        enableToggleBtn->toggle(b);
    if (callback)
        onEnabledChanged(this);

    onInfoChanged(true);
}

void GraphCell::deleteMe(){
    this->removeMeAndCleanup();
    
    onDeleted(graphInfo);
}