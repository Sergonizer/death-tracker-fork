#pragma once

#include <Geode/Geode.hpp>
#include <nodes/OptionsNode.hpp>
#include <types/DTTypes.hpp>
#include <nodes/SimpleToggler.hpp>


using namespace geode::prelude;

struct GetLevelsTaskProgress {
    std::string message;
    int current;
    int max;

    GetLevelsTaskProgress(std::string message_, int current_, int max_)
        : message(std::move(message_)), current(current_), max(max_) {}
};

struct LinkData {
    LevelMetadeta metadata;
    std::string levelKey;
};

using GetLevelsTask = Task<std::vector<LinkData>, GetLevelsTaskProgress>;

class LinkOptions : public OptionsNode {
    public:
        static LinkOptions* create(const CCSize& size);
    private:
        virtual bool setup();

        void getAllLevelsData();

        void onGetLevels(GetLevelsTask::Event* e);

        std::vector<LinkData> data{};
        EventListener<GetLevelsTask> getallLevelsListener;

        unsigned int currentLinkedPage;
        unsigned int currentPlayedPage;

        std::optional<unsigned int> maxLinkedPage;
        std::optional<unsigned int> maxPlayedPage;

        void updateScrollsContent();

        ScrollLayer* linkedScroll;
        ScrollLayer* playedScroll;

        SimpleTextArea* nonePlayedFoundText;
        SimpleTextArea* noneLinkedFoundText;

        static const int PER_PAGE_COUNT;

        void onLinkCellClicked(const std::string& levelKey, bool isLeftAligned);

        void onSwitchedPage(CCObject* sender);
        void updatePageLabels();
        CCLabelBMFont* playedScrollPageLabel;
        CCLabelBMFont* linkedScrollPageLabel;

        std::string searchStr;
        bool searchForID;
        TextInput* searchBar;

        void update(float dt);

        CCLabelBMFont* loadingLabel;
        LoadingCircle* loadingCircle;
        CCScale9Sprite* loadingShadow;

    public:
        virtual void onOpened();
        virtual void onClosed();
};