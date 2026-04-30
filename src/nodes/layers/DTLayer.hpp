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
template<class T>
using GetTFuture = arc::Future<Result<T, UpdateFutureError>>;

struct ColumnComperator {
    bool operator() (LayoutColumn* a, LayoutColumn* b) const;
};

struct organizationResult{
    std::vector<std::tuple<DTLabel*, CCPoint, float>> labelData{};
    float highestColumn;
};

using organizationFuture = arc::Future<organizationResult>;

class DTLayer : public Popup, public FLAlertLayerProtocol {
    protected:
        bool init(GJGameLevel* const& Level);

    public:
        static DTLayer* create(GJGameLevel* const& Level);

        void UpdateSharedStats();

        GJGameLevel* m_Level;

        Result<LevelData, ErrorWithCode> m_MyLevelStats = Err("");
        std::vector<LevelData> linkedLevelsData{};
        SessionCategory sessionsOrder{};
        SessionCategory daySGroup{};
        SessionCategory weekSGroup{};
        SessionCategory monthSGroup{};
        int currentGrouping = -4;
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

        Result<Session, UpdateFutureError> loadSessionFromSave(std::optional<int> sessionIndex = std::nullopt);

        void foreachLinkedLevel(geode::Function<void(LevelData&)> onLevelVisit);

        bool createDeathsString(const Deaths& deaths, const LevelMetadeta& meta, const stringCustomazations& custom, std::string& out, std::optional<NewBests> const newBests = std::nullopt, const ccColor3B& newBestColoring = {255, 255, 255}, bool ignoreExtraSettings = false);

        void CleanGetStats();

        template<class T>
        GetTFuture<T> getTFor(geode::Function<T(GeneralData const&)> dataGetter, geode::Function<T(T const&, T const&)> combineFunc, bool session, bool localOnly = false){
            if (!session){
                if (m_MyLevelStats.isErr()) co_return Err("Failed to calculate playtime");
                auto myStats = m_MyLevelStats.unwrap();
                if (myStats.from0.isErr()){
                    auto error = myStats.from0.unwrapErr();
                    if (error.code == 1)
                        co_return Err(UpdateFutureError("No Deaths Saved!", false));
                    else
                        co_return Err(error.error);
                }
                auto myFrom0Stats = myStats.from0.unwrap();
                auto linkedLevelsCopy = linkedLevelsData;

                T all = dataGetter(myFrom0Stats);

                if (!localOnly){
                    for (const auto& levelData : linkedLevelsCopy)
                    {
                        co_await arc::yield();
                        if (levelData.from0.isErr() || levelData.levelKey == myStats.levelKey) continue;
                        auto levelFrom0Stats = levelData.from0.unwrap();
                        all = combineFunc(all, dataGetter(levelFrom0Stats));
                    }
                }

                co_return Ok(all);
            }
            else{
                auto sessionRes = loadSessionFromSave();
                if (sessionRes.isErr()) co_return Err(sessionRes.unwrapErr());
                auto session = sessionRes.unwrap();

                co_await arc::yield();

                co_return Ok(dataGetter(session.data));
            }
        }

    private:
        
        std::mutex dataMutex;
        LayoutOptionsLayer* layoutOptionsLayer = nullptr;
        organizationFuture organizeLayoutTask();
        async::TaskHolder<organizationFuture::Output> organizationListener;
        async::TaskHolder<> orgCooldown;

        bool canOrganize = true;
        int firstTime = 0;
        LoadingCircle* lc;
        
        std::set<LayoutColumn*, ColumnComperator> columns;

        std::set<DTLabel*> keyListeners{};

        bool createDeathsString(const Deaths& deaths, const stringCustomazations& custom, std::string& out, std::optional<NewBests> const newBests = std::nullopt, const ccColor3B& newBestColoring = {255, 255, 255}, bool ignoreExtraSettings = false);

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

        LayoutColumn* addColumn(std::optional<DTColumnInfo> info = std::nullopt);
        void addColumnBtnClicked(CCObject*);

        std::map<CCNode*, geode::Function<void(float)>> onOrganizationCompleteEvent{};

        void setLayoutBy(const DTLayoutV3& layout);

        CCMenuItemSpriteExtra* addColumnButton;
        CCSprite* editLayoutBtnSpr;

        CCMenu* bottomLeftMenu;
        CCMenu* bottomRightMenu;
        CCMenu* editLayoutMenu;
        TutorialButton* layoutInfo;
        ButtonSprite* discardChangesButtonSpr;
        ButtonSprite* applyChangesButtonSpr;
        CCScale9Sprite* colorChangeBG;
        CCMenu* colorMenu;
        void onApplyLayoutChanges(CCObject*);
        void onDiscardLayoutChanges(CCObject*);

        void exitLayoutEditing();

        UpdateFuture onNLKey();
        UpdateFuture onATTKey();
        UpdateFuture onLVLNKey();
        UpdateFuture onGeneralKey();
        UpdateFuture onRUNSKey();
        UpdateFuture onS0Key();
        UpdateFuture onSRUNSKey();

        UpdateFuture onDTATTKey();
        UpdateFuture onDTF0ATTKey();
        UpdateFuture onDTRunsATTKey();

        UpdateFuture onSAttKey();
        UpdateFuture onSF0AttKey();
        UpdateFuture onSRunsAttKey();
        
        long long calcPlaytime(const Deaths& deaths);
        
        UpdateFuture onAPTALLSKey();
        UpdateFuture onAPTF0SKey();
        UpdateFuture onAPTRUNSKey();
        UpdateFuture onAPTSALLSKey();
        UpdateFuture onAPTSF0Key();
        UpdateFuture onAPTSRUNSKey();

        //playtime
        UpdateFuture onPTALLSKey();
        UpdateFuture onPTF0SKey();
        UpdateFuture onPTRUNSKey();
        UpdateFuture onPTSALLSKey();
        UpdateFuture onPTSF0Key();
        UpdateFuture onPTSRUNSKey();

        //dead playtime
        UpdateFuture onDeadPTALLSKey();
        UpdateFuture onDeadPTF0SKey();
        UpdateFuture onDeadPTRUNSKey();
        UpdateFuture onDeadPTSALLSKey();
        UpdateFuture onDeadPTSF0Key();
        UpdateFuture onDeadPTSRUNSKey();
        
        //paused playtime
        UpdateFuture onPausedPTALLSKey();
        UpdateFuture onPausedPTF0SKey();
        UpdateFuture onPausedPTRUNSKey();
        UpdateFuture onPausedPTSALLSKey();
        UpdateFuture onPausedPTSF0Key();
        UpdateFuture onPausedPTSRUNSKey();

        UpdateFuture onRunsTo100Key();
        UpdateFuture onBestRunsKey();

        UpdateFuture onSessionRunsTo100Key();
        UpdateFuture onSessionBestRunsKey();
        
        UpdateFuture onSectionKey();

        UpdateFuture onSessionDateKey();

        UpdateFuture onLevelRunsKey();

        UpdateFuture getPlaytimeFor(geode::Function<uint64_t(GeneralData const&)>&& dataGetter, bool session);
        UpdateFuture getAttemptsFor(geode::Function<unsigned long long(GeneralData const&, geode::FunctionRef<unsigned long long(const Deaths&)> const&)>&& dataGetter, bool session);

        std::optional<float> cachedLevelLength;

        void onCalculator(CCObject*);
        void onGroups(CCObject*);
        void onGroupSelected(int const& id);

        FloatingList* groupsList;
        CCMenuItemSpriteExtra* groupsBtn;
        CCMenu* groupsHolder;

        void onNewBestColor(CCObject*);
        void onSessionBestColor(CCObject*);

        CCMenu* resetLayoutMenu;
        void onResetLayout(CCObject*);

        void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override;

        CCSprite* newBestColorBtnSpr;
        CCSprite* sessionBestColorBtnSpr;

        ccColor3B originalNewBestColor = {255, 255, 255};
        ccColor3B originalSessionBestColor = {255, 255, 255};
};