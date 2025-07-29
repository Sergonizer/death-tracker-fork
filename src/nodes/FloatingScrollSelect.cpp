#include <nodes/FloatingScrollSelect.hpp>

#include <nodes/cells/FloatingSelectCell.hpp>

FloatingScrollSelect* FloatingScrollSelect::create(CCSize size, const std::vector<std::string>& options) {
    auto ret = new FloatingScrollSelect();
    if (ret && ret->init(size, options)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool FloatingScrollSelect::init(CCSize size, const std::vector<std::string>& options){
    if (!CCNode::init()) return false;

    this->setContentSize(size);

    scrollLayer = ScrollLayer::create(size, false);
    scrollLayer->m_contentLayer->setLayout(ColumnLayout::create()
        ->setAutoGrowAxis(scrollLayer->m_contentLayer->getContentHeight())
    );
    this->addChild(scrollLayer);

    for (const auto& option : options)
    {
        auto cell = FloatingSelectCell::create(size.width, option, [&](FloatingSelectCell* clickedCell){
            
        });
        scrollLayer->m_contentLayer->addChild(cell);
    }
    
    scrollLayer->m_contentLayer->updateLayout();

    return true;
}