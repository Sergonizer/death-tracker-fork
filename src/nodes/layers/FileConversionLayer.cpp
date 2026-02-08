#include <nodes/layers/FileConversionLayer.hpp>
#include <managers/StatsManager.hpp>

FileConversionLayer* FileConversionLayer::create() {
    auto popup = new FileConversionLayer;
    if (popup->init()) {
        popup->autorelease();
        return popup;
    }
    delete popup;
    return nullptr;
}

bool FileConversionLayer::init() {
    if (!Popup::init({250.f, 150.f}, "geode.loader/GE_square01.png"))
        return false;
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    this->setTitle("Conversion Layer");

    textArea = SimpleTextArea::create("");
    textArea->setPosition(m_size / 2);
    textArea->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    m_mainLayer->addChild(textArea);

    progressBar = Slider::create(nullptr, NULL);
    progressBar->m_touchLogic->setVisible(false);
    progressBar->m_touchLogic->setEnabled(false);
    progressBar->setValue(0);
    progressBar->setVisible(false);
    progressBar->setPosition(m_size / 2 - ccp(0, 35));
    m_mainLayer->addChild(progressBar);

    this->setKeypadEnabled(false);
    this->setKeyboardEnabled(false);
    this->m_closeBtn->setVisible(false);

    lookupListener.spawn(LookupTask{
        [](const LookupTask::ProgressFunc& progress) -> LookupTask::InnerFuture {
            progress(1);
            co_return StatsManager::allV2FileLevelKeys();
        },
        [&](std::weak_ptr<LookupTask::Progress> progress){
            if (auto strongRef = progress.lock()){
                if (*strongRef == 1){
                    this->setTitle("Working...");
                    textArea->setText("Looking for old save files...");
                }
            }
        }},
        std::bind(this, &FileConversionLayer::lookupComplete, std::placeholders::_1)
    );

    return true;
}

void FileConversionLayer::setTitle(const std::string& text){
    //geode::Popup<>::setTitle(text, "goldFont.fnt", .8f, 15);
}

void FileConversionLayer::lookupComplete(LookupTask::Result* result){
    if (!result->size()){
        this->setTitle("No old files found!");
        textArea->setText("It looks like you're all set! No old files were found!");
        return;
    }

    this->setTitle("Files found!");
    textArea->setText(fmt::format("<cy>{}</c> old files were found! Conversion will start right away!", result->size()));

    conversionListener.spawn(ConversionTask{
        [&, result](const ConversionTask::ProgressFunc& progress) -> ConversionTask::InnerFuture {
            int filesWentOverAmount = 0;
            int successfulConversionAmount = 0;
            std::vector<std::string> failedConversions{};

            for (const auto& levelKey : *result)
            {
                auto progressObj = ConversionProgress{};
                progressObj.filesWentOverAmount = filesWentOverAmount;
                progressObj.convertedAmount = successfulConversionAmount;
                progressObj.maxAmount = result->size();
                progressObj.currentConvertingFile = levelKey;

                progress(progressObj);

                auto conversionRes = StatsManager::convertV2SaveToV3(levelKey);

                if (conversionRes.isOk())
                    successfulConversionAmount++;
                else if (conversionRes.isErr()){
                    failedConversions.push_back(levelKey);
                }

                filesWentOverAmount++;
            }

            auto conversionRes = ConversionResult{};
            conversionRes.convertedAmount = successfulConversionAmount;
            conversionRes.maxAmount = result->size();
            conversionRes.failedConversions = failedConversions;

            co_return conversionRes;
        },
        [&](std::weak_ptr<ConversionTask::Progress> progress){
            if (auto progressRes = progress.lock()){
                progressBar->setVisible(true);
                float dev = static_cast<float>(progressRes->filesWentOverAmount) / static_cast<float>(progressRes->maxAmount);
                progressBar->setValue(dev);
                progressBar->updateBar();

                this->setTitle("Working...");
                textArea->setText(fmt::format(
                    "Converting files ({}/{})\nCurrently working on\n'{}'",
                    progressRes->convertedAmount,
                    progressRes->maxAmount,
                    progressRes->currentConvertingFile
                ));
            }
        }},
        std::bind(this, &FileConversionLayer::lookupComplete, std::placeholders::_1)
    );
    // conversionListener.bind(this, &FileConversionLayer::conversionComplete);
    // conversionListener.setFilter(FileConversionLayer::conversionStage(*result));
}

void FileConversionLayer::conversionComplete(ConversionTask::Result* resultRes){
    this->setTitle("Conversion complete!");
    textArea->setText(fmt::format(
        "Converted {}/{} files!\n{} files failed to convert.",
        resultRes->convertedAmount,
        resultRes->maxAmount,
        resultRes->failedConversions.size()
    ));

    progressBar->setVisible(true);
    progressBar->setValue(1);

    for (const auto& level : resultRes->failedConversions) log::error("Failed to convert level: {}", level);

    this->setKeypadEnabled(true);
    this->setKeyboardEnabled(true);
    this->m_closeBtn->setVisible(true);

    didComplete = true;
}

void FileConversionLayer::onClose(cocos2d::CCObject* sender){
    if (didComplete)
        Popup::onClose(sender);
}
