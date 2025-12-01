#include "LayoutOptionsLayer.hpp"
#include <nodes/layers/DTLayer.hpp>

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

    auto bg = CCScale9Sprite::create("geode.loader/GE_square01.png");
    bg->setContentSize(size);
    bg->setPosition(size / 2);
    this->addChild(bg);




    return true;
}

void LayoutOptionsLayer::setEditedNodeTo(DTLabel* label) {
    //setup the options layer to edit the given label
    editedLabel = label;
    editedColumn = std::nullopt;
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
}