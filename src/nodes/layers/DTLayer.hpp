#pragma once

#include <Geode/Geode.hpp>
#include "../../managers/StatsManager.hpp"
#include "../../utils/Save.hpp"
#include "AdvancedScrollLayer.hpp"
#include <nodes/SessionSelector.hpp>
#include <nodes/LayoutColumn.hpp>

using namespace geode::prelude;

using DeathStringTask = Task<Result<std::vector<DeathInfo>>>;
using ResultTask = Task<Result<>>;
using organizationTask = Task<std::vector<std::tuple<DTLabel*, CCPoint, float>>>;

class DTLayer : public Popup<GJGameLevel* const&> {
    protected:
        bool setup(GJGameLevel* const& Level) override;

    public:
        static DTLayer* create(GJGameLevel* const& Level);

        void UpdateSharedStats();

        void updateRunsAllowed();

        GJGameLevel* m_Level;

        Result<LevelData> m_MyLevelStats = Err("");
        std::vector<LevelData> linkedLevelsData{};
        std::multimap<long long, std::string, std::greater<long long>> sessionsOrder{};
        std::optional<Session> currentSessionInfo = std::nullopt;
        

        void show() override;

        virtual void keyDown(enumKeyCodes key)  override;
        virtual void keyUp(enumKeyCodes key)  override;

        static DTLayer* get();

        // DTLabel* createLabel(DTLabelInfo info = DTLabelInfo());

        // void removeLabel(DTLabel* label, bool forceDelete = false);

        AdvancedScrollLayer* getScrollLayer();
        
        virtual void onClose(CCObject*) override;
        virtual void keyBackClicked() override;

        // void saveAndUpdateStats(bool updateShared);

        // void UpdateOnAllShared(const std::function<void(LevelStats& stats)>& lambda);

        void UpdateDeathRelatedStrings();

        std::map<std::string, std::string> specialStrings{};

        void organizeLayout();
    private:
        organizationTask organizeLayoutTask();
        EventListener<organizationTask> organizationListener;
        
        struct ColumnComperator {
            bool operator() (LayoutColumn* a, LayoutColumn* b) const {
                return a->orderPos < b->orderPos;
            }
        };
        std::set<LayoutColumn*, ColumnComperator> columns;

        bool createDeathsString(const Deaths& deaths, const stringCustomazations& custom, std::string& out, NewBests* const newBests = nullptr, const std::string& newBestColoring = "");

        int currentSession;

        void update(float dt) override;

        void onSessionSelected(int sessionNum);

        bool ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent) override;
        void ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent) override;
        void ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent) override;
        void ccTouchCancelled(CCTouch *pTouch, CCEvent *pEvent) override;
        void ccTouchesMoved(CCSet* touches, CCEvent* event) override;

        static DTLayer* instance;

        AdvancedScrollLayer* scrollLayer = nullptr;
        // std::set<DTLabel*> labels{};
        // EditLayoutTopbar* layoutTopbar = nullptr;
        CCMenu* columnHolder;
        CCSize ogLimits;

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