#include "FloatingList.hpp"

FloatingList* FloatingList::create(CCSize const& size, bool startOpen){
    auto ret = new FloatingList();
    if (ret && ret->init(size, startOpen)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool FloatingList::init(CCSize const& size, bool startOpen){
    if (!CCNode::init()) return false;

    this->isOpen = startOpen;

    this->setContentSize(size);

    scrollLayer = ScrollLayer::create(size);
    this->addChild(scrollLayer);

    scrollLayer->m_contentLayer->setLayout(ColumnLayout::create()
        ->setGrowCrossAxis(true)
        ->setCrossAxisOverflow(false)
        ->setAutoGrowAxis(scrollLayer->getContentHeight())
        ->setAxisAlignment(AxisAlignment::End)
        ->setAxisReverse(true)
        ->ignoreInvisibleChildren(false)
        ->setGap(elementOffset)
    );
    scrollLayer->setScaleY(startOpen ? 1 : 0);

    auto listBG = CCScale9Sprite::create("square02_001.png");
    listBG->setContentSize(size);
    listBG->setOpacity(100);
    listBG->setPosition(size / 2);
    scrollLayer->addChild(listBG, -1);

    setOpenDirection(true);

    scrollLayer->setMouseEnabled(startOpen);
    scrollLayer->setStealingTouches(true);
    scrollLayer->setCancelTouchLimit(0.01f);
    scrollLayer->setKeypadEnabled(false);
    scrollLayer->setKeyboardEnabled(false);

    scrollbar = Scrollbar::create(scrollLayer);
    scrollbar->ignoreAnchorPointForPosition(false);
    scrollbar->setPositionX(scrollLayer->getContentWidth() - 4);
    scrollbar->setPositionY(scrollLayer->getContentHeight() / 2);
    scrollLayer->addChild(scrollbar);
    
    scrollbar->setTouchEnabled(startOpen);

    return true;
}

void FloatingList::addItem(const FloatingListItem& text){
    for (const auto& [_, item] : itemIds){
        if (item.id == text.id)
            return;
    }

    float off = 5;

    auto buttonSpr = ButtonSprite::create(text.text.c_str(), text.font.c_str(), text.BGTexture.c_str());
    float wantedScaleX = (this->getContentWidth() - off) / buttonSpr->getContentWidth();
    float wantedScaleY = (elementHeight - off) / buttonSpr->getContentHeight();
    buttonSpr->setScale(wantedScaleX > wantedScaleY ? wantedScaleY : wantedScaleX);
    auto buttonItem = CCMenuItemSpriteExtra::create(
        buttonSpr,
        this,
        menu_selector(FloatingList::itemClicked)
    );
    buttonItem->setTag(text.id);

    auto menu = CCMenu::createWithItem(buttonItem);
    menu->setContentSize({
        this->getContentWidth(),
        elementHeight
    });
    buttonItem->setPosition(menu->getContentSize() / 2);

    auto itemBG = CCScale9Sprite::create("square02_001.png");
    itemBG->setContentSize(menu->getContentSize());
    itemBG->setOpacity(150);
    itemBG->setPosition(menu->getContentSize() / 2);
    menu->addChild(itemBG, -1);
    
    buttonItem->setEnabled(isOpen);

    scrollLayer->m_contentLayer->addChild(menu);
    scrollLayer->m_contentLayer->updateLayout();
    itemIds[buttonItem] = text;
}
void FloatingList::addItems(const std::vector<FloatingListItem>& texts){
    for (const auto& text : texts){
        addItem(text);
    }
}

void FloatingList::open(){
    isOpen = true;

    scrollLayer->moveToTop();

    scrollLayer->stopAllActions();
    scrollLayer->runAction(CCEaseInOut::create(CCScaleTo::create(0.1f, 1, 1), 2));

    setItemEnabled(true);

    scrollLayer->setMouseEnabled(isOpen);
    scrollbar->setTouchEnabled(isOpen);
}

void FloatingList::close(){
    isOpen = false;

    scrollLayer->stopAllActions();
    scrollLayer->runAction(CCEaseInOut::create(CCScaleTo::create(0.1f, 1, 0), 2));

    setItemEnabled(false);

    scrollLayer->setMouseEnabled(isOpen);
    scrollbar->setTouchEnabled(isOpen);
}

void FloatingList::setItemEnabled(bool isEnabled){
    for (const auto& [item, _] : itemIds){
        item->setEnabled(isEnabled);
    }
}

void FloatingList::setCallback(geode::Function<void(const int& id)> callback){
    onItemClicked = std::move(callback);
}

void FloatingList::itemClicked(CCObject* sender){
    auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
    if (!itemIds.contains(btn)) return;

    if (onItemClicked) onItemClicked(itemIds[btn].id);
}

void FloatingList::setOpenDirection(bool openUpwards){
    if (openUpwards){
        scrollLayer->setAnchorPoint({0.5f, 0});
        scrollLayer->setPositionY(0);
    }
    else{
        scrollLayer->setAnchorPoint({0.5f, 1});
        scrollLayer->setPositionY(this->getContentHeight());
    }
}

std::optional<FloatingListItem> FloatingList::getItemForID(int id){
    for (const auto& [btn, item] : itemIds)
    {
        if (btn->getTag() == id)
            return item;
    }
    
    return std::nullopt;
}