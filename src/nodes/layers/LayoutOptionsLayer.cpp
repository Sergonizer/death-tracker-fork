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
    bg->setID("background");
    this->addChild(bg);

    auto generalBtnsMenu = CCMenu::create();
    generalBtnsMenu->setPosition({size.width / 2, size.height - 10});
    generalBtnsMenu->setContentSize({0, 0});
    generalBtnsMenu->setID("general-btns-menu");
    this->addChild(generalBtnsMenu);

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

    fontSelectionNode = CCMenu::create();
    fontSelectionNode->setPosition({size.width / 2, size.height - 10});
    fontSelectionNode->setContentSize({0, 0});
    fontSelectionNode->setID("font-selection-node");
    this->addChild(fontSelectionNode);

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
    labelNameInputLabel->setID("name-input-label");
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
    labelTextInputLabel->setID("text-color-label");
    labelSettingsNode->addChild(labelTextInputLabel);

    labelColorBtnSprite = CCSprite::createWithSpriteFrameName("GJ_colorBtn_001.png");
    labelColorBtnSprite->setID("color-spr");
    labelColorBtnSprite->setScale(.7f);
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
    labelColorBtnLabel->setID("label-color-label");
    labelSettingsNode->addChild(labelColorBtnLabel);

    textColorBtnSprite = CCSprite::createWithSpriteFrameName("GJ_colorBtn_001.png");
    textColorBtnSprite->setID("color-spr");
    textColorBtnSprite->setScale(.7f);
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
    textColorBtnLabel->setID("text-label");
    labelSettingsNode->addChild(textColorBtnLabel);

    auto colorLabel = CCLabelBMFont::create("color", "bigFont.fnt");
    colorLabel->setScale(.4f);
    colorLabel->setPosition({0, -80});
    colorLabel->setID("color-label");
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

        scaleSlider->setValue((newSize - DTLabelInfo::MIN_MAX_SCALE.x) / (DTLabelInfo::MIN_MAX_SCALE.y - DTLabelInfo::MIN_MAX_SCALE.x));
        editedLabel.value()->setFontSize(newSize);
    });
    fontSizeInput->setID("font-size-input");
    labelSettingsNode->addChild(fontSizeInput);

    auto fontSizeInputLabel = CCLabelBMFont::create("Font Size", "bigFont.fnt");
    fontSizeInputLabel->setScale(.4f);
    fontSizeInputLabel->setPosition(fontSizeInput->getPosition() + ccp(0, fontSizeInput->getScaledContentHeight() / 2 + fontSizeInputLabel->getScaledContentHeight() / 2));
    fontSizeInputLabel->setID("font-size-label");
    labelSettingsNode->addChild(fontSizeInputLabel);

    scaleSlider = Slider::create(this, menu_selector(LayoutOptionsLayer::scaleSliderChanged), .5f);
    scaleSlider->setPositionY(-215);
    scaleSlider->setID("font-size-slider");
    labelSettingsNode->addChild(scaleSlider);

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

    auto fontSelectionBtnSpr = ButtonSprite::create("Select Font");
    fontSelectionBtnSpr->setScale(.55f);
    auto fontSelectionBtn = CCMenuItemSpriteExtra::create(
        fontSelectionBtnSpr,
        this,
        menu_selector(LayoutOptionsLayer::onFontSelection)
    );
    fontSelectionBtn->setPositionY(-260);
    fontSelectionBtn->setID("font-selection-btn");
    labelSettingsNode->addChild(fontSelectionBtn);

    fontSelectedIndicatorLabel = SimpleTextArea::create("Selected: font -1", "gjFont21.fnt");
    fontSelectedIndicatorLabel->setID("font-select-indicator-label");
    fontSelectedIndicatorLabel->setScale(.4f);
    fontSelectedIndicatorLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    fontSelectedIndicatorLabel->setPosition(fontSelectionBtn->getPosition() + ccp(0, 5 + fontSelectionBtn->getScaledContentHeight() / 2 + fontSelectedIndicatorLabel->getScaledContentHeight() / 2));
    labelSettingsNode->addChild(fontSelectedIndicatorLabel);

    auto fontSelectionBtnLabel = CCLabelBMFont::create("Font", "bigFont.fnt");
    fontSelectionBtnLabel->setID("font-selection-label");
    fontSelectionBtnLabel->setScale(.4f);
    fontSelectionBtnLabel->setPosition(fontSelectedIndicatorLabel->getPosition() + ccp(0, fontSelectedIndicatorLabel->getScaledContentHeight() / 2 + fontSelectionBtnLabel->getScaledContentHeight() / 2));
    labelSettingsNode->addChild(fontSelectionBtnLabel);

    addEventListener<keybinds::InvokeBindFilter>([&](keybinds::InvokeBindEvent* event) {
        if (event->isDown() && labelTextInput->getInputNode()->m_selected && editedLabel.has_value()) {
            auto str = labelTextInput->getString();
            int pos = labelTextInput->getInputNode()->m_textField->m_uCursorPos;

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

    fontSelectionNode->setScaleY(0);
    labelSettingsNode->setScaleY(0);
    columnSettingsNode->setScaleY(0);

    auto backBtnSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    backBtnSpr->setScale(.65f);
    auto backBtn = CCMenuItemSpriteExtra::create(
        backBtnSpr,
        this,
        menu_selector(LayoutOptionsLayer::onBack)
    );
    backBtn->setID("back-btn");
    backBtn->setPosition({-75, 6});
    generalBtnsMenu->addChild(backBtn);
    
    auto deleteBtnSpr = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
    deleteBtnSpr->setScale(.65f);
    auto deleteBtn = CCMenuItemSpriteExtra::create(
        deleteBtnSpr,
        this,
        menu_selector(LayoutOptionsLayer::onDelete)
    );
    deleteBtn->setID("delete-btn");
    deleteBtn->setPosition({65, 6});
    generalBtnsMenu->addChild(deleteBtn);

    fontsScroll = ScrollLayer::create(size - ccp(8, 30));
    fontsScroll->setPosition({-size.width / 2, -size.height + 20});
    fontsScroll->m_contentLayer->setLayout(ColumnLayout::create()
        ->setGrowCrossAxis(true)
        ->setCrossAxisOverflow(false)
        ->setAutoGrowAxis(fontsScroll->getContentHeight())
        ->setAxisAlignment(AxisAlignment::End)
        ->setAxisReverse(true)
    );
    fontSelectionNode->addChild(fontsScroll);

    auto fontsScrollbar = Scrollbar::create(fontsScroll);
    fontsScrollbar->setPosition({68, fontsScroll->getPositionY() / 2});
    fontSelectionNode->addChild(fontsScrollbar);

    for (const auto& font : StatsManager::getAllFonts())
    {
        auto cell = FontSelectionCell::create(font, [&](auto cell){onFontSelected(cell);});
        fontsScroll->m_contentLayer->addChild(cell);

        allFontCells.insert({font, cell});
    }

    fontsScroll->m_contentLayer->updateLayout();
    fontsScroll->moveToTop();

    fontsScroll->setMouseEnabled(false);

    return true;
}

void LayoutOptionsLayer::setEditedNodeTo(DTLabel* label) {
    if (editedLabel.has_value() && editedLabel.value() == label) return;
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

    scaleSlider->setValue(
        (label->info.scale - DTLabelInfo::MIN_MAX_SCALE.x) / (DTLabelInfo::MIN_MAX_SCALE.y - DTLabelInfo::MIN_MAX_SCALE.x)
    );
    scaleSliderChanged(nullptr);

    fontSelectedIndicatorLabel->setText(fmt::format("selected: \"{}\"", label->info.font.substr(0, label->info.font.length() - 4)));
    fontSelectedIndicatorLabel->setFont(label->info.font);
    if (currentlySelectedFontCell != nullptr)
        currentlySelectedFontCell->deselect();
    currentlySelectedFontCell = allFontCells[label->info.font];
    currentlySelectedFontCell->select();

    switchToMenu(1);
}

void LayoutOptionsLayer::setEditedNodeTo(LayoutColumn* column) {
    if (editedColumn.has_value() && editedColumn.value() == column) return;
    editedColumn = column;
    editedLabel = std::nullopt;

    switchToMenu(2);
}

void LayoutOptionsLayer::close() {
    //close the options layer
    switchToMenu(0);
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

void LayoutOptionsLayer::scaleSliderChanged(CCObject*){
    auto numValue = DTLabelInfo::MIN_MAX_SCALE.x + (DTLabelInfo::MIN_MAX_SCALE.y - DTLabelInfo::MIN_MAX_SCALE.x) * scaleSlider->getValue();

    editedLabel.value()->setFontSize(numValue);

    auto scaleText = std::to_string(numValue);
    for (int i = scaleText.length() - 1; i >= 0; i--)
    {
        if (scaleText[i] != '0') break;

        scaleText.pop_back();
    }
    
    if (scaleText.length() != 0 && scaleText[scaleText.length() - 1] == '.') scaleText.pop_back();

    fontSizeInput->setString(scaleText);
}

void LayoutOptionsLayer::onFontSelection(CCObject*){
    switchToMenu(3);
}

void LayoutOptionsLayer::switchToMenu(uint8_t menuID){
    labelSettingsNode->stopAllActions();
    columnSettingsNode->stopAllActions();
    fontSelectionNode->stopAllActions();

    currentPage = menuID;

    fontsScroll->setMouseEnabled(false);

    auto exitEasing = [](bool open) -> CCEaseExponentialOut* {
        return CCEaseExponentialOut::create(CCScaleTo::create(.2f, 1, open ? 1 : 0));
    };

    labelSettingsNode->runAction(exitEasing(false));
    columnSettingsNode->runAction(exitEasing(false));
    fontSelectionNode->runAction(exitEasing(false));

    CCNode* selectedNode = nullptr;

    switch (menuID)
    {

    case 1:
        selectedNode = labelSettingsNode;
        break;

    case 2:
        selectedNode = columnSettingsNode;
        break;

    case 3:
        selectedNode = fontSelectionNode;
        fontsScroll->setMouseEnabled(true);
        fontsScroll->moveToTop();
        break;
    
    default:
        break;
    }

    if (selectedNode != nullptr){
        selectedNode->stopAllActions();
        selectedNode->runAction(CCSequence::create(
            exitEasing(false),
            CCDelayTime::create(.1f),
            exitEasing(true),
            nullptr
        ));
    }
}

void LayoutOptionsLayer::onBack(CCObject*){
    if (currentPage == 3){
        switchToMenu(editedLabel.has_value() ? 1 : 2);
        return;
    }

    if (onBackedOut != NULL)
        onBackedOut();
}

void LayoutOptionsLayer::onDelete(CCObject*){
    if (editedLabel.has_value()){
        editedLabel.value()->removeFromColumns();
        editedLabel.value()->removeFromParentAndCleanup(true);
        DTLayer::get()->organizeLayout();
        onBackedOut();
    }
    else if (editedColumn.has_value()){
        editedColumn.value()->destroyColumnAndCleanup();
        DTLayer::get()->organizeLayout();
        onBackedOut();
    }
}

void LayoutOptionsLayer::onFontSelected(FontSelectionCell* cell){
    if (!isEditingNode()) return;

    if (currentlySelectedFontCell != nullptr) currentlySelectedFontCell->deselect();
    currentlySelectedFontCell = cell;
    currentlySelectedFontCell->select();

    if (editedLabel.has_value()){
        editedLabel.value()->setFont(cell->font);
        DTLayer::get()->organizeLayout();
        fontSelectedIndicatorLabel->setText(fmt::format("selected: \"{}\"", cell->font.substr(0, cell->font.length() - 4)));
        fontSelectedIndicatorLabel->setFont(cell->font);
    }
}