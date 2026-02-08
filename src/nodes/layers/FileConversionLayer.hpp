#pragma once

#include <Geode/Geode.hpp>
#include <nodes/OptionsNode.hpp>
#include <utils/ProgressFuture.hpp>

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

using LookupTask = ProgressFuture<std::vector<std::string>, int>;
using ConversionTask = ProgressFuture<ConversionResult, ConversionProgress>;

class FileConversionLayer : public geode::Popup {
    public:
        static FileConversionLayer* create();
    private:
        bool init() override;

        SimpleTextArea* textArea;
        Slider* progressBar;

        void lookupComplete(LookupTask::Result* event);
        async::TaskHolder<LookupTask> lookupListener;

        void conversionComplete(ConversionTask::Result* event);
        async::TaskHolder<ConversionTask> conversionListener;
        
        void setTitle(const std::string& text);

        void onClose(cocos2d::CCObject*) override;

        bool didComplete;
};