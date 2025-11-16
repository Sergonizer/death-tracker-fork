#pragma once

#include <Geode/Geode.hpp>
#include <nodes/OptionsNode.hpp>

using namespace geode::prelude;

struct ConversionProgress{
    int maxAmount;
    int convertedAmount;
    int filesWentOverAmount;
    std::string currentConvertingFile;
};
struct ConversionResult{
    std::vector<std::string> failedConversions;
    int maxAmount;
    int convertedAmount;
};

using LookupTask = Task<std::vector<std::string>, int>;
using ConversionTask = Task<ConversionResult, ConversionProgress>;

class FileConversionLayer : public geode::Popup<> {
    public:
        static FileConversionLayer* create();
    private:
        bool setup() override;

        SimpleTextArea* textArea;
        Slider* progressBar;

        LookupTask lookupStage();
        void lookupComplete(LookupTask::Event* event);
        EventListener<LookupTask> lookupListener;

        ConversionTask conversionStage(const std::vector<std::string>& levelsToConvert);
        void conversionComplete(ConversionTask::Event* event);
        EventListener<ConversionTask> conversionListener;
        
        void setTitle(const std::string& text);

        void onClose(cocos2d::CCObject*) override;

        bool didComplete;
};