#include <nodes/EditLayoutTopbar.hpp>

#include <nodes/layers/DTLayer.hpp>

EditLayoutTopbar* EditLayoutTopbar::create() {
    auto ret = new EditLayoutTopbar();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool EditLayoutTopbar::init(){
    if (!CCNode::init()) return false;

    auto winSize = CCDirector::get()->getWinSize();

    this->setContentSize({winSize.width - 50, 40});

    auto BG = CCScale9Sprite::create("GJ_square01.png");
    BG->setContentSize(this->getContentSize());
    BG->setAnchorPoint({0, 0});
    this->addChild(BG);

    CCPoint menuOffset = {15, 15};

    idleMenu = CCMenu::create();
    idleMenu->setPosition(menuOffset / 2);
    idleMenu->setAnchorPoint({0, 0});
    idleMenu->setContentSize(this->getContentSize() - menuOffset);
    this->addChild(idleMenu);

    idleMenu->setLayout(SimpleAxisLayout::create(Axis::Row)
        ->setGap(2)
        ->setCrossAxisScaling(AxisScaling::ScaleDown)
    );

    auto exitBtnSpr = CCSprite::createWithSpriteFrameName("GJ_longBtn07_001.png");
    auto exitBtn = CCMenuItemSpriteExtra::create(
        exitBtnSpr,
        this,
        menu_selector(EditLayoutTopbar::onExitClicked)
    );
    idleMenu->addChild(exitBtn);

    auto applyBtnSpr = ButtonSprite::create("Apply");
    auto applyBtn = CCMenuItemSpriteExtra::create(
        applyBtnSpr,
        this,
        menu_selector(EditLayoutTopbar::onApplyClicked)
    );
    idleMenu->addChild(applyBtn);

    idleMenu->addChild(createSpacingNode(15));

    auto newLabelBtnSpr = CCSprite::createWithSpriteFrameName("GJ_plus3Btn_001.png");
    auto newLabelBtn = CCMenuItemSpriteExtra::create(
        newLabelBtnSpr,
        this,
        menu_selector(EditLayoutTopbar::onNewLabelClicked)
    );
    idleMenu->addChild(newLabelBtn);

    idleMenu->updateLayout();

    //

    targetMenu = CCMenu::create();
    targetMenu->setPosition(menuOffset / 2);
    targetMenu->setAnchorPoint({0, 0});
    targetMenu->setContentSize(this->getContentSize() - menuOffset);
    targetMenu->setVisible(false);
    targetMenu->setEnabled(false);
    this->addChild(targetMenu);

    targetMenu->setLayout(SimpleAxisLayout::create(Axis::Row)
        ->setGap(2)
        ->setCrossAxisScaling(AxisScaling::ScaleDown)
    );

    auto deleteBtnSpr = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
    auto deleteBtn = CCMenuItemSpriteExtra::create(
        deleteBtnSpr,
        this,
        menu_selector(EditLayoutTopbar::onDeleteClicked)
    );
    targetMenu->addChild(deleteBtn);

    targetMenu->addChild(createSpacingNode(15));

    auto fontSelBtnSpr = ButtonSprite::create("F");
    auto fontSelBtn = CCMenuItemSpriteExtra::create(
        fontSelBtnSpr,
        this,
        menu_selector(EditLayoutTopbar::onFontSelClicked)
    );
    targetMenu->addChild(fontSelBtn);

    targetMenu->addChild(createSpacingNode(10));

    HAlignmentBtnSpr = ButtonSprite::create("hA");
    auto HAlignmentBtn = CCMenuItemSpriteExtra::create(
        HAlignmentBtnSpr,
        this,
        menu_selector(EditLayoutTopbar::onHAlignmentClicked)
    );
    targetMenu->addChild(HAlignmentBtn);

    VAlignmentBtnSpr = ButtonSprite::create("vA");
    auto VAlignmentBtn = CCMenuItemSpriteExtra::create(
        VAlignmentBtnSpr,
        this,
        menu_selector(EditLayoutTopbar::onVAlignmentClicked)
    );
    targetMenu->addChild(VAlignmentBtn);

    targetMenu->addChild(createSpacingNode(10));

    colorBtnSpr = CCSprite::createWithSpriteFrameName("GJ_colorBtn_001.png");
    auto colorBtn = CCMenuItemSpriteExtra::create(
        colorBtnSpr,
        this,
        menu_selector(EditLayoutTopbar::onColorClicked)
    );
    targetMenu->addChild(colorBtn);

    targetMenu->addChild(createSpacingNode(10));

    scaleInput = TextInput::create(30, "S", "gjFont17.fnt");
    scaleInput->setCommonFilter(CommonFilter::Float);
    scaleInput->setCallback([&](const std::string& newText){
        auto numRes = geode::utils::numFromString<float>(newText);

        if (numRes.isErr()) return;

        auto num = numRes.unwrap();

        num = std::clamp(num, 0.1f, 2.5f);

        targetLabel->labelInfo.scale = num;
        targetLabel->updateTransform();
    });
    targetMenu->addChild(scaleInput);

    targetMenu->addChild(createSpacingNode(10));

    contentWidthInput = TextInput::create(30, "W", "gjFont17.fnt");
    contentWidthInput->setCommonFilter(CommonFilter::Float);
    contentWidthInput->setCallback([&](const std::string& newText){
        auto numRes = geode::utils::numFromString<float>(newText);

        if (numRes.isErr()) return;

        auto num = numRes.unwrap();

        num = std::max(num, 0.1f);

        targetLabel->labelInfo.contentSize.width = num;
        targetLabel->updateText();
    });
    targetMenu->addChild(contentWidthInput);

    auto seperator = CCLabelBMFont::create("-", "gjFont17.fnt");
    targetMenu->addChild(seperator);

    contentHeightInput = TextInput::create(30, "H", "gjFont17.fnt");
    contentHeightInput->setCommonFilter(CommonFilter::Float);
    contentHeightInput->setCallback([&](const std::string& newText){
        auto numRes = geode::utils::numFromString<float>(newText);

        if (numRes.isErr()) return;

        auto num = numRes.unwrap();

        num = std::max(num, 0.1f);

        targetLabel->labelInfo.contentSize.height = num;
        targetLabel->updateText();
    });
    targetMenu->addChild(contentHeightInput);

    targetMenu->addChild(createSpacingNode(10));

    auto checkOff = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
    auto checkOn = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
    infGrowBtn = CCMenuItemToggler::create(
        checkOff,
        checkOn,
        this,
        menu_selector(EditLayoutTopbar::onInfGrowClicked)
    );
    targetMenu->addChild(infGrowBtn);

    auto textBtnSpr = ButtonSprite::create("T");
    auto textBtn = CCMenuItemSpriteExtra::create(
        textBtnSpr,
        this,
        menu_selector(EditLayoutTopbar::onTextClicked)
    );
    targetMenu->addChild(textBtn);

    targetMenu->addChild(createSpacingNode(15));

    auto deselectBtnSpr = CCSprite::createWithSpriteFrameName("GJ_deSelBtn2_001.png");
    auto deselectBtn = CCMenuItemSpriteExtra::create(
        deselectBtnSpr,
        this,
        menu_selector(EditLayoutTopbar::onDeselectClicked)
    );
    targetMenu->addChild(deselectBtn);

    targetMenu->updateLayout();

    textMenu = CCMenu::create();
    textMenu->setPosition(menuOffset / 2);
    textMenu->setAnchorPoint({0, 0});
    textMenu->setContentSize(this->getContentSize() - menuOffset);
    textMenu->setVisible(false);
    this->addChild(textMenu);

    auto textConfirmBtnSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
    auto textConfirmBtn = CCMenuItemSpriteExtra::create(
        textConfirmBtnSpr,
        this,
        menu_selector(EditLayoutTopbar::onConfirmTextClicked)
    );
    textMenu->addChild(textConfirmBtn);

    textInput = TextInput::create(winSize.width, "Text", "gjFont17.fnt");
    textInput->setCommonFilter(CommonFilter::Any);
    textInput->setCallback([&](const std::string& newText){
        targetLabel->labelInfo.text = newText;

        targetLabel->updateText();
    });
    textMenu->addChild(textInput);

    auto spcialKeysBtnSpr = CCSprite::createWithSpriteFrameName("GJ_bigGoldKey_001.png");
    auto spcialKeysBtn = CCMenuItemSpriteExtra::create(
        spcialKeysBtnSpr,
        this,
        menu_selector(EditLayoutTopbar::onSpecialKeysClicked)
    );
    textMenu->addChild(spcialKeysBtn);

    textMenu->setLayout(SimpleAxisLayout::create(Axis::Row)
        ->setGap(6)
        ->setCrossAxisScaling(AxisScaling::ScaleDown)
    );

    textMenu->updateLayout();

    return true;
}

void EditLayoutTopbar::setTarget(DTLabel* target){
    if (target == this->targetLabel) return;
    if (this->targetLabel != nullptr) onDeselectClicked(nullptr);
    this->targetLabel = target;

    targetMenu->setVisible(true);
    targetMenu->setEnabled(true);

    idleMenu->setVisible(false);
    idleMenu->setEnabled(false);

    colorBtnSpr->setColor({targetLabel->labelInfo.color.r, targetLabel->labelInfo.color.g, targetLabel->labelInfo.color.b});
    colorBtnSpr->setOpacity(targetLabel->labelInfo.color.a);

    scaleInput->setString(fmt::format("{:.2f}", targetLabel->labelInfo.scale));
    contentWidthInput->setString(fmt::format("{:.2f}", targetLabel->labelInfo.contentSize.width));
    contentHeightInput->setString(fmt::format("{:.2f}", targetLabel->labelInfo.contentSize.height));

    textInput->setString(targetLabel->labelInfo.text);

    infGrowBtn->toggle(targetLabel->labelInfo.infinityResize);
}

void EditLayoutTopbar::onApplyClicked(CCObject*){
    if (onExit != NULL)
        onExit(true);
    this->removeMeAndCleanup();
}

void EditLayoutTopbar::onExitClicked(CCObject*){
    if (onExit != NULL)
        onExit(false);
    this->removeMeAndCleanup();
}

void EditLayoutTopbar::onNewLabelClicked(CCObject*){
    auto dtlayer = DTLayer::get();
    if (dtlayer == nullptr) return;

    auto newLabel = dtlayer->createLabel();

    auto gridPos = newLabel->localToGridPosition(dtlayer->getScrollLayer()->content->getContentSize() / 2 - dtlayer->getScrollLayer()->content->getPosition());

    newLabel->labelInfo.X = gridPos.x;
    newLabel->labelInfo.Y = gridPos.y;

    newLabel->updateState();

    setTarget(newLabel);
}


void EditLayoutTopbar::onFontSelClicked(CCObject* sender){
    auto fontScroll = FloatingScrollSelect::create({75, 200}, {"f", "f", "f", "f", "f", "f", "f", "f", "f", "f", "f"});
    fontScroll->setPosition(static_cast<CCNode*>(sender)->getPosition());
    this->addChild(fontScroll);
    // + ccp(-size.width / 2, this->getContentHeight() / 2)
}

void EditLayoutTopbar::onHAlignmentClicked(CCObject*){
    auto alignment = targetLabel->labelInfo.horizontalAlignment;

    switch (alignment)
    {
    case kCCTextAlignmentCenter:
        alignment = kCCTextAlignmentRight;
        break;
    case kCCTextAlignmentRight:
        alignment = kCCTextAlignmentLeft;
        break;
    case kCCTextAlignmentLeft:
        alignment = kCCTextAlignmentCenter;
        break;
    
    default:
        break;
    }

    targetLabel->labelInfo.horizontalAlignment = alignment;

    targetLabel->updateText();
}

void EditLayoutTopbar::onVAlignmentClicked(CCObject*){
    auto alignment = targetLabel->labelInfo.verticalAlignment;

    switch (alignment)
    {
    case kCCTextAlignmentCenter:
        alignment = kCCTextAlignmentRight;
        break;
    case kCCTextAlignmentRight:
        alignment = kCCTextAlignmentLeft;
        break;
    case kCCTextAlignmentLeft:
        alignment = kCCTextAlignmentCenter;
        break;
    
    default:
        break;
    }

    targetLabel->labelInfo.verticalAlignment = alignment;

    targetLabel->updateText();
}

void EditLayoutTopbar::onColorClicked(CCObject*){
    auto picker = geode::ColorPickPopup::create(targetLabel->labelInfo.color);
    picker->setDelegate(this);
    picker->show();
}

void EditLayoutTopbar::updateColor(cocos2d::ccColor4B const& color){
    targetLabel->labelInfo.color = color;
    colorBtnSpr->setColor({color.r, color.g, color.b});
    colorBtnSpr->setOpacity(color.a);

    targetLabel->updateText();
}

void EditLayoutTopbar::onInfGrowClicked(CCObject* sender){
    auto toggler = static_cast<CCMenuItemToggler*>(sender);

    targetLabel->labelInfo.infinityResize = !toggler->isToggled();
    targetLabel->updateText();
}

void EditLayoutTopbar::onDeselectClicked(CCObject*){
    this->targetLabel->onDeselected();
    this->targetLabel = nullptr;

    idleMenu->setVisible(true);
    idleMenu->setEnabled(true);

    targetMenu->setVisible(false);
    targetMenu->setEnabled(false);

    textMenu->setVisible(false);
}

void EditLayoutTopbar::onDeleteClicked(CCObject*){
    auto dtlayer = DTLayer::get();
    if (dtlayer == nullptr || targetLabel == nullptr) return;

    auto toDelete = targetLabel;

    onDeselectClicked(nullptr);
    dtlayer->removeLabel(toDelete);
}

void EditLayoutTopbar::onTextClicked(CCObject*){
    textMenu->setVisible(true);
    targetMenu->setVisible(false);
}

CCNode* EditLayoutTopbar::createSpacingNode(float spacing){
    auto spacingNode = CCNode::create();
    spacingNode->setContentSize({spacing, spacing});
    return spacingNode;
}

void EditLayoutTopbar::keyBackClicked(){
    if (textMenu->isVisible()){
        textMenu->setVisible(false);
        targetMenu->setVisible(true);
    }
    else if (targetLabel != nullptr)
        onDeselectClicked(nullptr);
    else
        onExitClicked(nullptr);
}

void EditLayoutTopbar::onConfirmTextClicked(CCObject*){
    keyBackClicked();
}
void EditLayoutTopbar::onSpecialKeysClicked(CCObject*){
    
}