#pragma once

using namespace geode::prelude;

using UpdateTask = geode::Task<Result<std::string>, int>;

class SpecialKey : CCObject {
    private:
        std::string currentContent;

        std::string key;

        std::optional<UpdateTask> updateFunction = std::nullopt;

        std::function<void(SpecialKey*)> updateCompletedCallback = NULL;
        std::function<void(SpecialKey*)> updateStartedCallback = NULL;

        geode::EventListener<UpdateTask> updateListener;

        void onUpdateCompleted(UpdateTask::Event* e);

        bool updateOngoing = false;

    public:
        SpecialKey(const std::string& key);

        bool compareToKey(const std::string& otherKey);

        void setUpdateFunction(const UpdateTask& task);

        void updateContent();

        std::string getContent() const{
            return this->currentContent;
        }
        std::string getKey() const{
            return this->key;
        }

        void setUpdateCompletedCallback(const std::function<void(SpecialKey*)>& callback);
        void setUpdateStartedCallback(const std::function<void(SpecialKey*)>& callback);
};