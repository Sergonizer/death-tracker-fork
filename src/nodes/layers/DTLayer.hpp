#pragma once

#include <Geode/Geode.hpp>
#include "../../managers/StatsManager.hpp"
#include "../../utils/Save.hpp"
#include "AdvancedScrollLayer.hpp"

using namespace geode::prelude;

using DeathStringTask = Task<Result<std::vector<DeathInfo>>>;
using ResultTask = Task<Result<>>;

class DTLayer : public Popup<GJGameLevel* const&> {
    protected:
        bool setup(GJGameLevel* const& Level) override;

    public:
        static DTLayer* create(GJGameLevel* const& Level);

        //updates the shared stats, the shared stats will contain all deaths and runs from all linked levels including the current one
        void UpdateSharedStats();

        //saves and refreshes the main page text
        void updateRunsAllowed();

        GJGameLevel* m_Level;

        LevelStats m_MyLevelStats;
        LevelStats m_SharedLevelStats;

        bool m_IsClicking;
        CCTouch* ClickPos = nullptr;

        void show() override;

        virtual void keyDown(enumKeyCodes key)  override;
        virtual void keyUp(enumKeyCodes key)  override;
        
    private:
        virtual void keyBackClicked() override;

        AdvancedScrollLayer* scrollLayer = nullptr;

        //better info time calc :)

        uint64_t timeInMs();
        std::string decodeBase64Gzip(const std::string& input);
        float timeForLevelString(const std::string& levelString);

        //linking

        //opens the link menu
        void OnLinkButtonClicked(CCObject*);

        //edit layout mode
        
        //toggles the edit layout menu on
        void onEditLayout(CCObject*);

        //general

        bool ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent) override;
        void ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent) override;
        void ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent) override;
        void ccTouchCancelled(CCTouch *pTouch, CCEvent *pEvent) override;

        //graph

        //opens the graph menu
        void openGraphMenu(CCObject*);

        //settings

        //opens the geode setting menu for the mod
        void onSettings(CCObject*);

        //info about the labels and edit layout menu
        void onLayoutInfo(CCObject*);
        //info about copying label text
        void onCopyInfo(CCObject*);
};