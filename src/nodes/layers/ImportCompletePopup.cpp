#include "ImportCompletePopup.hpp"
#include <nodes/layers/DTLayer.hpp>

ImportCompletePopup* ImportCompletePopup::create(LevelData const& data, std::vector<Session> const& sessions) {
    auto ret = new ImportCompletePopup();
    if (ret->init(std::move(data), std::move(sessions))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ImportCompletePopup::init(LevelData _data, std::vector<Session> const& _sessions) {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    if (!Popup::init(250, 150, "geode.loader/GE_square01.png"))
        return false;
    setTitle("Import Complete");

    this->data = std::move(_data);
    this->sessions = std::move(_sessions);

    

    return true;
}