#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

enum TutorialCharacterFace{
    TCFNormal
};

struct TutorialSegment{
    DialogObject* dialogue = nullptr;
    DialogChatPlacement alignment = DialogChatPlacement::Center;
    float boxScale = 1;

    std::set<CCNode*> targetObjects{};
};

class DTTutorialLayer : public CCLayer, public DialogDelegate {
    public:
        static DTTutorialLayer* create();

        DTTutorialLayer* appendDialogue(
            const std::string& text,
            TutorialCharacterFace face,
            float textSize = 1,
            const ccColor3B& textColor = {255, 255, 255},
            float boxScale = 1,
            DialogChatPlacement alignment = DialogChatPlacement::Center
        );
        DTTutorialLayer* appendDialogue(DialogObject* dialogue, float boxScale = 1, DialogChatPlacement alignment = DialogChatPlacement::Center);
        DTTutorialLayer* joinHighlight(CCNode* targetObject);

        void show();
        
    private:
        bool init();
        
        std::vector<TutorialSegment> allSegments{};
        std::map<DialogObject*, int> dialogueSegmentIndexes{};
        CCLayerColor* shadow;

        CCNode* highlightsHolder;

        CCRenderTexture* lightRT;

        void onProgress(DialogObject* dObject);

        DialogLayer* dialogueLayer;

        void dialogClosed(DialogLayer* layer);

        void close();

        bool firstDialogue = false;
};