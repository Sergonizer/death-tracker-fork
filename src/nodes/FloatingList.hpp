#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

struct FloatingListItem {
    int id;

    std::string text;
    std::string font = "bigFont.fnt";
    std::string BGTexture = "GJ_button_01.png";
};

class FloatingList : public CCNode, public CCTouchDelegate {
    public:
        static FloatingList* create(CCSize const& size, bool startOpen = false);

        void addItem(const FloatingListItem& text);
        void addItems(const std::vector<FloatingListItem>& texts);

        void open();
        void close();

        void setCallback(geode::Function<void(const int& id)> callback);

        void setOpenDirection(bool openUpwards);

        bool isOpened() const { return isOpen; }

        std::optional<FloatingListItem> getItemForID(int id);

    private:
        bool init(CCSize const& size, bool startOpen);

        geode::Function<void(const int& id)> onItemClicked;

        void itemClicked(CCObject* sender);

        void setItemEnabled(bool isEnabled);

        bool ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent) override;

        std::map<CCMenuItemSpriteExtra*, FloatingListItem> itemIds{};

        ScrollLayer* scrollLayer;
        Scrollbar* scrollbar;

        float elementHeight = 20.f;

        float elementOffset = 5;

        bool isOpen = false;

        CCNode* clickArea;

        void onEnter() override;
        void onExit() override;
};