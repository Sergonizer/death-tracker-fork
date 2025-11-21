#pragma once

#include <Geode/Geode.hpp>
#include "../../managers/StatsManager.hpp"
#include "../../utils/Save.hpp"
#include "AdvancedScrollLayer.hpp"
#include <nodes/SessionSelector.hpp>
#include <nodes/LayoutColumn.hpp>
#include <types/DTTypes.hpp>

using namespace geode::prelude;

struct ColumnComperator {
    bool operator() (LayoutColumn* a, LayoutColumn* b) const;
};

struct organizationResult{
    std::vector<std::tuple<DTLabel*, CCPoint, float>> labelData{};
    float highestColumn;
};

using DeathStringTask = Task<Result<std::vector<DeathInfo>>>;
using ResultTask = Task<Result<>>;
using organizationTask = Task<organizationResult>;

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

        bool doMoveScroll = true;

        std::pair<LayoutColumn*, int> getColumnLayerFromPosition(CCPoint posInWorldSpace);

        //points should be in world space!
        std::multiset<LayoutColumn*, ColumnComperator> getColumnsBetween(CCPoint a, CCPoint b);

        DTLabel* createNewLabel(DTLabelInfo info = DTLabelInfo{});

        void subscribeToOrganizationEvent(CCNode* target, const std::function<void(float)>& callback);
        void unsubscribeToOrganizationEvent(CCNode* target);

        void fixUpColumnPositions();

        static float transitionTime;

    private:
        organizationTask organizeLayoutTask();
        EventListener<organizationTask> organizationListener;
        
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

        CCNode* labelsHolder;

        SessionSelector* sessionSelector;

        void onLSOClicked(CCObject*);

        void onEditLayout(CCObject*);

        void graphBtnClicked(CCObject*);

        void onSettings(CCObject*);

        //better info time calc :)
        uint64_t timeInMs();
        std::string decodeBase64Gzip(const std::string& input);
        float timeForLevelString(const std::string& levelString);

        LayoutColumn* addColumn(std::optional<DTColumnInfo> info = std::nullopt);
        void addColumnBtnClicked(CCObject*);

        std::map<CCNode*, std::function<void(float)>> onOrganizationCompleteEvent{};

        void setLayoutBy(const DTLayoutV3& layout);
        void saveCurrentLayout();
};