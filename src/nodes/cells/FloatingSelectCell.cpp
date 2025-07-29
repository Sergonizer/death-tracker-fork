#include "../cells/FloatingSelectCell.hpp"

FloatingSelectCell* FloatingSelectCell::create(float width, const std::string& optionText, const std::function<void(FloatingSelectCell*)>& callback) {
    auto ret = new FloatingSelectCell();
    if (ret && ret->init(width, optionText, callback)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool FloatingSelectCell::init(float width, const std::string& optionText, const std::function<void(FloatingSelectCell*)>& callback){
    if (!CCNode::init()) return false;

    this->setContentSize({width, 35});

    auto BG = CCScale9Sprite::create("square02c_001.png");
    BG->setContentSize(this->getContentSize());
    BG->setAnchorPoint({0, 0});
    this->addChild(BG);

    return true;
}
