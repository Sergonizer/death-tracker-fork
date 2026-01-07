#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

enum TutorialBoxPlacement{
    TBPTop,
    TBPBottom,
    TBPLeft,
    TBPRight,
    TBPCenter,
    TBPBottomRight,
    TBPTopRight,
    TBPTopLeft,
    TBPBottomLeft,
};
enum TutorialTextPlacement{
    TTTop,
    TTBottom,
    TTLeft,
    TTRight,
    TTCenter
};

enum TutorialCharacterFace{
    TCFNormal
};

struct HighlightText{
    std::string text;
    TutorialTextPlacement placement;
    float size;
};

struct TutorialSegment{
    DialogObject* dialogue = nullptr;
    std::optional<TutorialBoxPlacement> alignment = std::nullopt;
    std::optional<float> boxScale = std::nullopt;

    CCNode* lastAddedHighlight;

    std::map<CCNode*, float> targetObjects{};
    std::map<CCNode*, HighlightText> textForTargets{};
};

class DTTutorialLayer : public CCLayer, public DialogDelegate {
    public:
        static DTTutorialLayer* create();

        DTTutorialLayer* appendDialogue(
            const std::string& text, 
            TutorialCharacterFace face, 
            const ccColor3B& textColor = {255,255,255}, 
            float textSize = 1
        );
        DTTutorialLayer* appendDialogue(DialogObject* dialogue);
        DTTutorialLayer* joinTransform(TutorialBoxPlacement alignment, float boxScale = 1);
        DTTutorialLayer* joinHighlight(CCNode* targetObject, float delayTime = 0);
        DTTutorialLayer* joinTextToHighlight(const std::string& text, float size = 1, TutorialTextPlacement alignment = TutorialTextPlacement::TTTop);

        void show();

        ~DTTutorialLayer();
        
    private:
        bool init();
        
        std::vector<TutorialSegment> allSegments{};
        std::map<DialogObject*, int> dialogueSegmentIndexes{};
        CCLayerColor* shadow;

        CCRenderTexture* lightRT;

        void onProgress(DialogObject* dObject);

        DialogLayer* dialogueLayer;

        void dialogClosed(DialogLayer* layer);

        void close();

        bool firstDialogue = false;

        std::set<CCScale9Sprite*> retainedSprites{};

        void update(float dt);
        std::map<CCScale9Sprite*, CCNode*> prevHighlights{};
        std::map<CCScale9Sprite*, CCLabelBMFont*> textsForHighlights{};
};