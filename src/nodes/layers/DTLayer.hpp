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
#include <nodes/TutorialButton.hpp>
#include <nodes/FloatingList.hpp>

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
        SessionCategory sessionsOrder{};
        SessionCategory daySGroup{};
        SessionCategory weekSGroup{};
        SessionCategory monthSGroup{};
        int currentGrouping = -3;
        SessionCategory& getCurrentGrouping();


        void show() override;

        virtual void keyDown(enumKeyCodes key, double d)  override;
        virtual void keyUp(enumKeyCodes key, double d)  override;

        static DTLayer* get();

        AdvancedScrollLayer* getScrollLayer();
        
        virtual void onClose(CCObject*) override;
        virtual void keyBackClicked() override;

        void UpdateDeathRelatedStrings();
        void updateStaticGroupings();

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

        void subscribeToOrganizationEvent(CCNode* target, geode::Function<void(float)> callback);
        void unsubscribeToOrganizationEvent(CCNode* target);

        void fixUpColumnPositions();

        static float transitionTime;

        void setOptionsLayerTo(DTLabel* label);
        void setOptionsLayerTo(LayoutColumn* column);
        void closeOptionsLayer();

        void removeColumn(LayoutColumn* column);
        bool cornerOnNextOrganization = true;
        void saveCurrentLayout();
        bool isEditingLayout = false;

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

        void foreachLinkedLevel(geode::Function<void(LevelData&)> onLevelVisit);

    private:
        
        LayoutOptionsLayer* layoutOptionsLayer = nullptr;
        organizationFuture organizeLayoutTask();
        async::TaskHolder<organizationFuture::Output> organizationListener;
        async::TaskHolder<> orgCooldown;

        bool canOrganize = true;
        int firstTime = 0;
        LoadingCircle* lc;
        
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

        void transferPlaytimeFromPT();

        //better info time calc :)
        uint64_t timeInMs();
        std::string decodeBase64Gzip(const std::string& input);
        float timeForLevelString(const std::string& levelString);
        int stoi(std::string_view str);
        float stof(std::string_view str);

        LayoutColumn* addColumn(std::optional<DTColumnInfo> info = std::nullopt);
        void addColumnBtnClicked(CCObject*);

        std::map<CCNode*, geode::Function<void(float)>> onOrganizationCompleteEvent{};

        void setLayoutBy(const DTLayoutV3& layout);

        CCMenuItemSpriteExtra* addColumnButton;
        CCSprite* editLayoutBtnSpr;

        CCMenu* bottomMenu;
        CCMenu* editLayoutMenu;
        TutorialButton* layoutInfo;
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
        
        UpdateFuture onAPTALLSKey();
        UpdateFuture onAPTF0SKey();
        UpdateFuture onAPTRUNSKey();
        UpdateFuture onAPTSALLSKey();
        UpdateFuture onAPTSF0Key();
        UpdateFuture onAPTSRUNSKey();


        UpdateFuture onPTALLSKey();
        UpdateFuture onPTF0SKey();
        UpdateFuture onPTRUNSKey();
        UpdateFuture onPTSALLSKey();
        UpdateFuture onPTSF0Key();
        UpdateFuture onPTSRUNSKey();

        UpdateFuture onRunsTo100Key();
        UpdateFuture onBestRunsKey();
        UpdateFuture onSAttKey();
        
        UpdateFuture onSectionKey();

        UpdateFuture onSessionDateKey();

        UpdateFuture getPlaytimeFor(geode::Function<uint64_t(GeneralData const&)>&& dataGetter, bool session);

        std::optional<float> cachedLevelLength;

        void onCalculator(CCObject*);
};