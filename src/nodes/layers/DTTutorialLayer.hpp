#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

struct TutorialSegment{
    DialogObject* dialogue = nullptr;
    DialogChatPlacement alignment = DialogChatPlacement::Center;

    std::set<CCNode*> targetObjects{};
};

class DTTutorialLayer : public CCLayer, public DialogDelegate {
    public:
        static DTTutorialLayer* create();

        DTTutorialLayer* appendDialogue(DialogObject* dialogue, DialogChatPlacement alignment = DialogChatPlacement::Center);
        DTTutorialLayer* joinHighlight(CCNode* targetObject);

        void show();
        
    private:
        bool init();
        
        std::vector<TutorialSegment> allSegments{};
        std::map<DialogObject*, int> dialogueSegmentIndexes{};
        CCLayerColor* shadow;

        CCNode* highlightsHolder;

        CCRenderTexture* darknessRT;
        CCRenderTexture* lightRT;

        void onProgress(DialogObject* dObject);

        DialogLayer* dialogueLayer;

        void dialogClosed(DialogLayer* layer);

        void close();
};