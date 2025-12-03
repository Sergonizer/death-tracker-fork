#include "LayoutOptionsLayer.hpp"
#include <nodes/layers/DTLayer.hpp>
#include <hooks/DTCCTextFieldTTF.hpp>
#include <geode.custom-keybinds/include/Keybinds.hpp>

/*
settings needed:

- for labels:
    - labels name (string) :D
    - text color (ccColor3B) :D
    - font size (float) :XD
    - label color (ccColor3B) :D
    - text (string) :D
    - alignment (CCTextAlignment) :D
    - special key selection (other UI)
    - font
    - movement and expanding options maybe

- for columns:
    - column width (float)
    - column position (float)
    - color (ccColor3B)

- for both:
    - delete (button)
*/

LayoutOptionsLayer* LayoutOptionsLayer::create(const CCSize& size) {
    auto ret = new LayoutOptionsLayer();
    if (ret && ret->init(size)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool LayoutOptionsLayer::init(const CCSize& size) {
    if (!CCLayer::init()) return false;

    this->setContentSize(size);

    // @geode-ignore(unknown-resource)
    auto bg = CCScale9Sprite::create("geode.loader/GE_square01.png");
    bg->setContentSize(size);
    bg->setPosition(size / 2);
    this->addChild(bg);

    labelSettingsNode = CCMenu::create();
    labelSettingsNode->setPosition({size.width / 2, size.height - 10});
    labelSettingsNode->setContentSize({0, 0});
    labelSettingsNode->setID("label-settings-node");
    this->addChild(labelSettingsNode);

    columnSettingsNode = CCMenu::create();
    columnSettingsNode->setPosition({size.width / 2, size.height - 10});
    columnSettingsNode->setContentSize({0, 0});
    columnSettingsNode->setID("column-settings-node");
    this->addChild(columnSettingsNode);

    labelNameInput = TextInput::create((size.width - 10) / .75f, "label name");
    labelNameInput->setID("label-name-input");
    labelNameInput->setScale(.75f);
    labelNameInput->setCallback([&](const std::string& newStr){
        if (!editedLabel.has_value()) return;

        editedLabel.value()->setLabelName(newStr);
    });
    labelNameInput->setPositionY(-22);
    labelSettingsNode->addChild(labelNameInput);

    auto labelNameInputLabel = CCLabelBMFont::create("Label Name", "bigFont.fnt");
    labelNameInputLabel->setScale(.4f);
    labelNameInputLabel->setPosition(labelNameInput->getPosition() + ccp(0, labelNameInput->getScaledContentHeight() / 2 + labelNameInputLabel->getScaledContentHeight() / 2));
    labelSettingsNode->addChild(labelNameInputLabel);

    labelTextInput = TextInput::create((size.width - 10) / .75f, "label text");
    labelTextInput->setScale(.75f);
    labelTextInput->setCommonFilter(CommonFilter::Any);
    labelTextInput->setID("label-text-input");
    labelTextInput->setPositionY(-60);
    labelTextInput->setCallback([&](const std::string& newStr){
        if (!editedLabel.has_value()) return;

        editedLabel.value()->setLabelText(newStr);
    });
    labelSettingsNode->addChild(labelTextInput);

    auto labelTextInputLabel = CCLabelBMFont::create("Text", "bigFont.fnt");
    labelTextInputLabel->setScale(.4f);
    labelTextInputLabel->setPosition(labelTextInput->getPosition() + ccp(0, labelTextInput->getScaledContentHeight() / 2 + labelTextInputLabel->getScaledContentHeight() / 2));
    labelSettingsNode->addChild(labelTextInputLabel);

    labelColorBtnSprite = CCSprite::createWithSpriteFrameName("GJ_colorBtn_001.png");
    labelColorBtnSprite->setID("color-spr");
    labelColorBtnSprite->setScale(.7f);
    // labelColorBtnSprite->setColor({
    //     editedLabel.value()->info.labelColor.r,
    //     editedLabel.value()->info.labelColor.g,
    //     editedLabel.value()->info.labelColor.b,
    // });
    auto labelColorBtn = CCMenuItemSpriteExtra::create(
        labelColorBtnSprite,
        this,
        menu_selector(LayoutOptionsLayer::onLabelColorBtnClicked)
    );
    labelColorBtn->setID("label-color-btn");
    labelColorBtn->setPosition({-30, -110});
    labelSettingsNode->addChild(labelColorBtn);

    auto labelColorBtnLabel = CCLabelBMFont::create("label", "bigFont.fnt");
    labelColorBtnLabel->setScale(.4f);
    labelColorBtnLabel->setPosition(labelColorBtn->getPosition() + ccp(0, labelColorBtn->getScaledContentHeight() / 2 + labelColorBtnLabel->getScaledContentHeight() / 2));
    labelSettingsNode->addChild(labelColorBtnLabel);

    textColorBtnSprite = CCSprite::createWithSpriteFrameName("GJ_colorBtn_001.png");
    textColorBtnSprite->setID("color-spr");
    textColorBtnSprite->setScale(.7f);
    // textColorBtnSprite->setColor({
    //     editedLabel.value()->info.textColor.r,
    //     editedLabel.value()->info.textColor.g,
    //     editedLabel.value()->info.textColor.b,
    // });
    auto textColorBtn = CCMenuItemSpriteExtra::create(
        textColorBtnSprite,
        this,
        menu_selector(LayoutOptionsLayer::onTextColorBtnClicked)
    );
    textColorBtn->setID("text-color-btn");
    textColorBtn->setPosition({30, -110});
    labelSettingsNode->addChild(textColorBtn);

    auto textColorBtnLabel = CCLabelBMFont::create("text", "bigFont.fnt");
    textColorBtnLabel->setScale(.4f);
    textColorBtnLabel->setPosition(textColorBtn->getPosition() + ccp(0, textColorBtn->getScaledContentHeight() / 2 + textColorBtnLabel->getScaledContentHeight() / 2));
    labelSettingsNode->addChild(textColorBtnLabel);

    auto colorLabel = CCLabelBMFont::create("color", "bigFont.fnt");
    colorLabel->setScale(.4f);
    colorLabel->setPosition({0, -80});
    labelSettingsNode->addChild(colorLabel);

    fontSizeInput = TextInput::create((size.width - 60) / .75f, "size");
    fontSizeInput->setScale(.75f);
    fontSizeInput->setPositionY(-195);
    fontSizeInput->setCommonFilter(CommonFilter::Float);
    fontSizeInput->setCallback([&](const std::string& newStr){
        if (!editedLabel.has_value()) return;
        
        auto newSizeRes = geode::utils::numFromString<float>(newStr);
        float newSize = 0.0f;

        if (!newSizeRes.isErr()) newSize = newSizeRes.unwrap();

        editedLabel.value()->setFontSize(newSize);
    });
    fontSizeInput->setID("font-size-input");
    labelSettingsNode->addChild(fontSizeInput);

    auto fontSizeInputLabel = CCLabelBMFont::create("Font Size", "bigFont.fnt");
    fontSizeInputLabel->setScale(.4f);
    fontSizeInputLabel->setPosition(fontSizeInput->getPosition() + ccp(0, fontSizeInput->getScaledContentHeight() / 2 + fontSizeInputLabel->getScaledContentHeight() / 2));
    labelSettingsNode->addChild(fontSizeInputLabel);

    alignmentMenu = CCMenu::create();
    alignmentMenu->setPosition({0, -155});
    alignmentMenu->setContentSize({133, 28});
    alignmentMenu->setID("alignment-menu");
    labelSettingsNode->addChild(alignmentMenu);
    alignmentMenu->ignoreAnchorPointForPosition(false);
    alignmentMenu->setLayout(SimpleAxisLayout::create(Axis::Row)
        ->setGap(5)
        ->setMainAxisScaling(AxisScaling::ScaleDown)
        ->setCrossAxisScaling(AxisScaling::ScaleDown)
        ->setMainAxisAlignment(MainAxisAlignment::Center)
    );

    auto alignmentLabel = CCLabelBMFont::create("Alignment", "bigFont.fnt");
    alignmentLabel->setScale(.4f);
    alignmentLabel->setPosition(alignmentMenu->getPosition() + ccp(0, 5 + alignmentMenu->getScaledContentHeight() / 2 + alignmentLabel->getScaledContentHeight() / 2));
    labelSettingsNode->addChild(alignmentLabel);

    auto leftAlignBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("alignLeft.png"_spr),
        this,
        menu_selector(LayoutOptionsLayer::onAlignmentChanged)
    );
    leftAlignBtn->setID("left-align-btn");
    alignmentMenu->addChild(leftAlignBtn);

    auto centerAlignBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("alignCenter.png"_spr),
        this,
        menu_selector(LayoutOptionsLayer::onAlignmentChanged)
    );
    centerAlignBtn->setID("center-align-btn");
    alignmentMenu->addChild(centerAlignBtn);

    auto rightAlignBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("alignRight.png"_spr),
        this,
        menu_selector(LayoutOptionsLayer::onAlignmentChanged)
    );
    rightAlignBtn->setID("right-align-btn");
    alignmentMenu->addChild(rightAlignBtn);

    alignmentMenu->updateLayout();

    

    addEventListener<keybinds::InvokeBindFilter>([&](keybinds::InvokeBindEvent* event) {
        if (event->isDown() && labelTextInput->getInputNode()->m_selected && editedLabel.has_value()) {
            auto str = labelTextInput->getString();
            int pos = labelTextInput->getInputNode()->m_textField->m_uCursorPos;

            log::info("plaing at {}", pos);

            if (pos == -1) str += "{nl}";
            else {
                str = str.insert(pos, "{nl}");
                labelTextInput->getInputNode()->m_textField->m_uCursorPos += 4;
            }

            labelTextInput->setString(str);
            editedLabel.value()->setLabelText(labelTextInput->getString());
        }
        return ListenerResult::Propagate;
    }, "enter-new-line"_spr);

    return true;
}

void LayoutOptionsLayer::setEditedNodeTo(DTLabel* label) {
    //setup the options layer to edit the given label
    editedLabel = label;
    editedColumn = std::nullopt;

    if (!label->info.isExpanded)
        label->toggleExpand(nullptr);

    onAlignmentChanged(alignmentMenu->getChildByID(fmt::format(
        "{}-align-btn",
        label->info.horizontalAlignment == CCTextAlignment::kCCTextAlignmentLeft ? "left" : (
            label->info.horizontalAlignment == CCTextAlignment::kCCTextAlignmentCenter ? "center" : "right"
        )
    )));

    labelColorBtnSprite->setColor({label->info.labelColor.r, label->info.labelColor.g, label->info.labelColor.b});
    textColorBtnSprite->setColor({label->info.textColor.r, label->info.textColor.g, label->info.textColor.b});

    labelNameInput->setString(label->info.labelName);
    labelTextInput->setString(label->info.text);

    auto scaleText = std::to_string(label->info.scale);
    for (int i = scaleText.length() - 1; i >= 0; i--)
    {
        if (scaleText[i] != '0') break;

        scaleText.pop_back();
    }
    
    if (scaleText.length() != 0 && scaleText[scaleText.length() - 1] == '.') scaleText.pop_back();

    fontSizeInput->setString(scaleText);
}

void LayoutOptionsLayer::setEditedNodeTo(LayoutColumn* column) {
    //setup the options layer to edit the given column
    editedColumn = column;
    editedLabel = std::nullopt;
}

void LayoutOptionsLayer::close() {
    //close the options layer
    editedLabel = std::nullopt;
    editedColumn = std::nullopt;
    colorChangeFunc = NULL;
}

void LayoutOptionsLayer::onTextColorBtnClicked(CCObject*){
    if (!editedLabel.has_value()) return;

    auto popup = geode::ColorPickPopup::create(editedLabel.value()->info.textColor);
    popup->setDelegate(this);
    popup->setColorTarget(textColorBtnSprite);
    colorChangeFunc = [&](cocos2d::ccColor4B const& color) {
        editedLabel.value()->setTextColor(color);
    };
    popup->show();
}
void LayoutOptionsLayer::onLabelColorBtnClicked(CCObject*){
    if (!editedLabel.has_value()) return;

    auto popup = geode::ColorPickPopup::create(editedLabel.value()->info.labelColor);
    popup->setDelegate(this);
    popup->setColorTarget(labelColorBtnSprite);
    colorChangeFunc = [&](cocos2d::ccColor4B const& color) {
        editedLabel.value()->setLabelColor(color);
    };
    popup->show();
}

void LayoutOptionsLayer::updateColor(cocos2d::ccColor4B const& color){
    if (colorChangeFunc == NULL) return;

    colorChangeFunc(color);
}

void LayoutOptionsLayer::onAlignmentChanged(CCObject* sender){
    if (!editedLabel.has_value()) return;

    auto menuItem = static_cast<CCMenuItemSpriteExtra*>(sender);

    for (const auto& alignmentButton : CCArrayExt<CCMenuItemSpriteExtra*>(alignmentMenu->getChildren()))
    {
        alignmentButton->setOpacity(255);
        alignmentButton->setEnabled(true);
    }

    CCTextAlignment alignment;


    if (menuItem->getID() == "left-align-btn") {
        alignment = kCCTextAlignmentLeft;
    } else if (menuItem->getID() == "center-align-btn") {
        alignment = kCCTextAlignmentCenter;
    } else if (menuItem->getID() == "right-align-btn") {
        alignment = kCCTextAlignmentRight;
    } else {
        return;
    }

    menuItem->setOpacity(100);
    menuItem->setEnabled(false);

    editedLabel.value()->setTextAlignment(alignment);
}