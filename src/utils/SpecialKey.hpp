#pragma once

using namespace geode::prelude;

using UpdateTask = geode::Task<Result<std::string>, int>;

class SpecialKey : public CCObject, public std::enable_shared_from_this<SpecialKey> {
    private:
        std::string currentContent;

        std::string key;
        std::string description;

        std::optional<std::function<UpdateTask()>> updateFunction = std::nullopt;

        std::function<void(const std::shared_ptr<SpecialKey>&)> updateCompletedCallback = nullptr;
        std::function<void(const std::shared_ptr<SpecialKey>&)> updateStartedCallback = nullptr;

        geode::EventListener<UpdateTask> updateListener;

        void onUpdateCompleted(UpdateTask::Event* e);

        bool updateOngoing = false;

    public:
        SpecialKey(const std::string& key, const std::string& description);
        ~SpecialKey(){
            //log::info("destroyed special key {}", key);
        }

        bool compareToKey(const std::string& otherKey);

        void setUpdateFunction(const std::function<UpdateTask()>& task);

        void updateContent();

        std::string getContent() const{
            return this->currentContent;
        }
        std::string getKey() const{
            return this->key;
        }
        std::string getDescription() const{
            return this->description;
        }

        void setUpdateCompletedCallback(const std::function<void(const std::shared_ptr<SpecialKey>&)>& callback);
        void setUpdateStartedCallback(const std::function<void(const std::shared_ptr<SpecialKey>&)>& callback);
};