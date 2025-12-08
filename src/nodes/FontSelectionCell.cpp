#include "FontSelectionCell.hpp"

FontSelectionCell* FontSelectionCell::create(const std::string& font, const std::function<void(FontSelectionCell* onSelected)>& onFontChosen){
    auto ret = new FontSelectionCell();
    if (ret && ret->init(font, onFontChosen)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool FontSelectionCell::init(const std::string& font, const std::function<void(FontSelectionCell* onSelected)>& onFontChosen){
    if (!CCMenu::init()) return false;

    this->setContentSize({135, 60});
    this->setPosition({0, 0});
    this->ignoreAnchorPointForPosition(false);

    auto bg = CCScale9Sprite::create("GJ_square01.png");
    bg->setID("background");
    bg->setContentSize(this->getContentSize());
    bg->setAnchorPoint({0, 0});
    bg->setOpacity(150);
    this->addChild(bg);

    this->font = font;
    this->onFontChosen = onFontChosen;

    return true;
}

void FontSelectionCell::deselect(){

}

void FontSelectionCell::select(){

}