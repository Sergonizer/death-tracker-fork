#pragma once

#include <Geode/Geode.hpp>
#include "../../managers/StatsManager.hpp"
#include "../../utils/Save.hpp"

using namespace geode::prelude;

using DeathStringTask = Task<Result<std::vector<DeathInfo>>>;
using ResultTask = Task<Result<>>;

class DTLayer : public Popup<GJGameLevel* const&>, public TextInputDelegate, public FLAlertLayerProtocol, public ColorPickPopupDelegate {
    protected:
        bool setup(GJGameLevel* const& Level) override;

    public:
        static DTLayer* create(GJGameLevel* const& Level);

        //will look for any special keys in the given string and output a formatted string accordingly
        std::string modifyString(std::string ToModify);

        //updates the shared stats, the shared stats will contain all deaths and runs from all linked levels including the current one
        void UpdateSharedStats();

        //activates when clicking a window
        //
        //currently only used for copying the windows text
        void clickedWindow(CCNode* window);

        //changes the size of the main scroll layer depending on the layout boxes added
        void changeScrollSizeByBoxes(const bool& moveToTop = false);

        //saves and refreshes the main page text
        void updateRunsAllowed();

        //refreshes all the text, this includes all run and death infos, strings, session strings and the main pages labels
        //this whole operation happens on a task in the background and activates the refresh circle until finished
        void refreshAll(bool moveToTop = false);
        //refreshes session related stuff, this includes session run and death infos, session strings and the main pages labels
        //this whole operation happens on a task in the background and activates the refresh circle until finished
        void refreshSession(bool moveToTop = false);

        void onClose(cocos2d::CCObject*) override;

        GJGameLevel* m_Level;

        LevelStats m_MyLevelStats;
        LevelStats m_SharedLevelStats;

        bool m_IsClicking;
        CCTouch* ClickPos = nullptr;

    private:
        void update(float delta) override;

        void updateColor(cocos2d::ccColor4B const& color) override;

        //text

        //update the displayed text
        void RefreshText(bool moveToTop = false);

        //gets and processes the saved deaths for the current level, outputs result into 'm_DeathsInfo'
        DeathStringTask CreateDeathsInfo(const Deaths& deaths, const NewBests& newBests);
        //gets and processes the saved runs for the current level, outputs result into 'm_RunInfo'
        DeathStringTask CreateRunsInfo(const Runs runs);
        //uses the deaths and runs info to create a long string to be displayed in the main page
        ResultTask refreshStrings();
        //uses the deaths and runs info to create a long string to be displayed in the main page, this creates the string specifically for the current session selected
        ResultTask updateSessionString(const int& session);

        //updates the playtime based on the given deaths and the current levels length
        //@param deaths the deaths to mesure from
        //@param runs wether to update the runs playtime or from 0 playime
        void updatePlaytime(std::vector<DeathInfo> deaths, bool runs);

        //better info time calc :)

        uint64_t timeInMs();
        std::string decodeBase64Gzip(const std::string& input);
        float timeForLevelString(const std::string& levelString);

        //session selection
        
        //switches the session selected one to the right
        void SwitchSessionRight(CCObject*);
        //switches the session selected one to the left
        void SwitchSessionLeft(CCObject*);

        //linking

        //opens the link menu
        void OnLinkButtonClicked(CCObject*);

        //edit layout mode
        
        //toggles the edit layout menu on
        void onEditLayout(CCObject*);
        
        //activates when clicking the apply button on the edit layout menu, will save all changes to the main layout
        void onEditLayoutApply(CCObject*);

        //toggles edit layout menu on or off
        void EditLayoutEnabled(const bool& b);
        
        //creates the layout label blocks representing the text, and deletes the old one
        void createLayoutBlocks();
        
        //adds a new layout label
        void addBox(CCObject*);
        
        //resets the layout to the default layout
        void onResetLayout(CCObject*);
        //opens the color picker layer for editing the new bests color
        void editnbcColor(CCObject*);
        //opens the color picker layer for editing the session bests color
        void editsbcColor(CCObject*);

        //general

        bool ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent) override;
        void ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent) override;
        void ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent) override;
        void ccTouchCancelled(CCTouch *pTouch, CCEvent *pEvent) override;

        void textChanged(CCTextInputNode* input) override;
        void textInputOpened(CCTextInputNode* input) override;
        void textInputClosed(CCTextInputNode* input) override;

        void FLAlert_Clicked(FLAlertLayer* layer, bool selected) override;

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

        //copy

        //opens the edit layout menu and modifies it to let you click a label box to copy its text
        void copyText(CCObject*);

        //specific settings

        //opens the level specific settings layer
        void onSpecificSettings(CCObject*);
        //handle the main and specific settings layer after their enter/exit transition finishes
        void onMoveTransitionEnded(CCObject* LSSL);
        //disables the refresh circle once a refresh process is finished
        void onRefreshFinished(ResultTask::Event* event);

        //data

        bool m_noSavedData;
        
        //main page
};