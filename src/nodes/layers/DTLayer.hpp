#pragma once

#include <Geode/Geode.hpp>
#include "../../managers/StatsManager.hpp"
#include "../../utils/Save.hpp"
#include "AdvancedScrollLayer.hpp"
#include <nodes/SessionSelector.hpp>
#include <nodes/LayoutColumn.hpp>
#include <types/DTTypes.hpp>
#include <utils/SpecialKey.hpp>
#include <nodes/layers/LayoutOptionsLayer.hpp>

using namespace geode::prelude;

struct ColumnComperator {
    bool operator() (LayoutColumn* a, LayoutColumn* b) const;
};

struct organizationResult{
    std::vector<std::tuple<DTLabel*, CCPoint, float>> labelData{};
    float highestColumn;
};

using organizationFuture = arc::Future<organizationResult>;

class DTLayer : public Popup {
    protected:
        bool init(GJGameLevel* const& Level);

    public:
        static DTLayer* create(GJGameLevel* const& Level);

        void UpdateSharedStats();

        GJGameLevel* m_Level;

        Result<LevelData> m_MyLevelStats = Err("");
        std::vector<LevelData> linkedLevelsData{};
        std::map<long long, std::string, std::greater<long long>> sessionsOrder{};

        void show() override;

        virtual void keyDown(enumKeyCodes key, double d)  override;
        virtual void keyUp(enumKeyCodes key, double d)  override;

        static DTLayer* get();

        AdvancedScrollLayer* getScrollLayer();
        
        virtual void onClose(CCObject*) override;
        virtual void keyBackClicked() override;

        void UpdateDeathRelatedStrings();

        std::map<std::string, std::shared_ptr<SpecialKey>> specialStrings{};
        void addSpecialString(const std::shared_ptr<SpecialKey>& key);
        void populateSpecialStrings();

        void specialKeyUpdateStarted(const std::shared_ptr<SpecialKey>& key);
        void specialKeyUpdateCompleted(const std::shared_ptr<SpecialKey>& key);

        void organizeLayout();

        bool doMoveScroll = true;

        std::pair<LayoutColumn*, int> getColumnLayerFromPosition(CCPoint posInWorldSpace);

        //points should be in world space!
        std::set<LayoutColumn*, ColumnComperator> getColumnsBetween(CCPoint a, CCPoint b);

        DTLabel* createNewLabel(DTLabelInfo info = DTLabelInfo{});

        void subscribeToOrganizationEvent(CCNode* target, const std::function<void(float)>& callback);
        void unsubscribeToOrganizationEvent(CCNode* target);

        void fixUpColumnPositions();

        static float transitionTime;

        void setOptionsLayerTo(DTLabel* label);
        void setOptionsLayerTo(LayoutColumn* column);
        void closeOptionsLayer();

        void removeColumn(LayoutColumn* column);
        bool cornerOnNextOrganization;
        void saveCurrentLayout();
        bool isEditingLayout;

        void subscribeKeyListener(DTLabel* label);
        void unsubscribeKeyListener(DTLabel* label);

        int getCurrentSelectedSession();
        void onSessionSelected(int sessionNum, bool updateContent);

        void modifyRun(int startPer, int amount, std::optional<int> sessionNumber);
        void modifyRun(int startPer, int endPer, int amount, std::optional<int> sessionNumber);
        void modifyNewBest(int percent, bool makeTrue, std::optional<int> sessionNumber);

        bool DeleteSave();

        SessionSelector* sessionSelector = nullptr;

        Result<Session> loadSessionFromSave(std::optional<int> sessionIndex = std::nullopt);

        void foreachLinkedLevel(const std::function<void(LevelData&)>& onLevelVisit);

    private:
        
        LayoutOptionsLayer* layoutOptionsLayer;
        organizationFuture organizeLayoutTask();
        async::TaskHolder<organizationFuture::Output> organizationListener;
        
        std::set<LayoutColumn*, ColumnComperator> columns;

        std::set<DTLabel*> keyListeners{};

        bool createDeathsString(const Deaths& deaths, const stringCustomazations& custom, std::string& out, NewBests* const newBests = nullptr, const std::string& newBestColoring = "", bool ignoreExtraSettings = false);

        int currentSession;

        void update(float dt) override;

        bool ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent) override;
        void ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent) override;
        void ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent) override;
        void ccTouchCancelled(CCTouch *pTouch, CCEvent *pEvent) override;
        void ccTouchesMoved(CCSet* touches, CCEvent* event) override;

        static DTLayer* instance;

        AdvancedScrollLayer* scrollLayer = nullptr;
        CCMenu* columnHolder;
        CCSize ogLimits;

        CCNode* labelsHolder;

        void onLSOClicked(CCObject*);

        void onEditLayout(CCObject*);

        void graphBtnClicked(CCObject*);

        void onSettings(CCObject*);

        //better info time calc :)
        uint64_t timeInMs();
        std::string decodeBase64Gzip(const std::string& input);
        float timeForLevelString(const std::string& levelString);
        int stoi(std::string_view str);
        float stof(std::string_view str);
        std::string workingTime(int value);

        LayoutColumn* addColumn(std::optional<DTColumnInfo> info = std::nullopt);
        void addColumnBtnClicked(CCObject*);

        std::map<CCNode*, std::function<void(float)>> onOrganizationCompleteEvent{};

        void setLayoutBy(const DTLayoutV3& layout);

        CCMenuItemSpriteExtra* addColumnButton;
        CCSprite* editLayoutBtnSpr;

        CCMenu* bottomMenu;
        CCMenu* editLayoutMenu;
        ButtonSprite* discardChangesButtonSpr;
        ButtonSprite* applyChangesButtonSpr;
        void onApplyLayoutChanges(CCObject*);
        void onDiscardLayoutChanges(CCObject*);

        void exitLayoutEditing();

        UpdateFuture onNLKey();
        UpdateFuture onATTKey();
        UpdateFuture onLVLNKey();
        UpdateFuture onGeneralKey();
        UpdateFuture onDTATTKey();
        UpdateFuture onRUNSKey();
        UpdateFuture onS0Key();
        UpdateFuture onSRUNSKey();
        
        long long calcPlaytime(const Deaths& deaths);
        
        UpdateFuture onPTALLSKey();
        UpdateFuture onPTF0SKey();
        UpdateFuture onPTRUNSKey();
        UpdateFuture onPTSALLSKey();
        UpdateFuture onPTSF0Key();
        UpdateFuture onPTSRUNSKey();

        UpdateFuture onRunsTo100Key();
        UpdateFuture onBestRunsKey();
        UpdateFuture onSAttKey();

        std::optional<float> cachedLevelLength;
};