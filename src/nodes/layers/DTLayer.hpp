#pragma once

#include <Geode/Geode.hpp>
#include "../../managers/StatsManager.hpp"
#include "../../utils/Save.hpp"
#include "AdvancedScrollLayer.hpp"
#include <nodes/DTLabel.hpp>
#include <nodes/EditLayoutTopbar.hpp>
#include <nodes/SessionSelector.hpp>

using namespace geode::prelude;

using DeathStringTask = Task<Result<std::vector<DeathInfo>>>;
using ResultTask = Task<Result<>>;

class DTLayer : public Popup<GJGameLevel* const&> {
    protected:
        bool setup(GJGameLevel* const& Level) override;

    public:
        static DTLayer* create(GJGameLevel* const& Level);

        void UpdateSharedStats();

        void updateRunsAllowed();

        GJGameLevel* m_Level;

        Result<LevelStats> m_MyLevelStats = Err("");
        Result<LevelStats> m_SharedLevelStats = Err("");

        void show() override;

        virtual void keyDown(enumKeyCodes key)  override;
        virtual void keyUp(enumKeyCodes key)  override;

        static DTLayer* get();

        DTLabel* createLabel(DTLabelInfo info = DTLabelInfo());

        void removeLabel(DTLabel* label, bool forceDelete = false);

        AdvancedScrollLayer* getScrollLayer();
        
        virtual void onClose(CCObject*) override;
        virtual void keyBackClicked() override;
    private:

        bool ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent) override;
        void ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent) override;
        void ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent) override;
        void ccTouchCancelled(CCTouch *pTouch, CCEvent *pEvent) override;
        void ccTouchesMoved(CCSet* touches, CCEvent* event) override;

        static DTLayer* instance;

        AdvancedScrollLayer* scrollLayer = nullptr;

        std::set<DTLabel*> labels{};

        EditLayoutTopbar* layoutTopbar = nullptr;

        SessionSelector* sessionSelector;

        void onLSOClicked(CCObject*);

        void onEditLayout(CCObject*);

        void graphBtnClicked(CCObject*);

        void onSettings(CCObject*);

        //better info time calc :)
        uint64_t timeInMs();
        std::string decodeBase64Gzip(const std::string& input);
        float timeForLevelString(const std::string& levelString);
};