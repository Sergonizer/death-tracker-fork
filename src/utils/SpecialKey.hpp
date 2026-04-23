#pragma once

using namespace geode::prelude;

using UpdateFuture = arc::Future<Result<std::string>>;

#ifndef BIND_UPDATE_FUNC
#define BIND_UPDATE_FUNC(METHOD) [&]() { return METHOD(); }
#endif

class SpecialKey : public CCObject, public std::enable_shared_from_this<SpecialKey> {
    private:
        std::string currentContent;

        std::string key;
        std::string description;

        std::optional<geode::Function<UpdateFuture()>> updateFunction = std::nullopt;

        geode::Function<void(const std::shared_ptr<SpecialKey>&)> updateCompletedCallback = nullptr;
        geode::Function<void(const std::shared_ptr<SpecialKey>&)> updateStartedCallback = nullptr;

        async::TaskHolder<UpdateFuture::Output> updateListener;

        UpdateFuture updateFutureRunner();
        void onUpdateCompleted(UpdateFuture::Output val);

        bool updateOngoing = false;

        std::set<std::string> toRefreshWith{};

    public:
        SpecialKey(const std::string& key, const std::string& description);
        ~SpecialKey(){
            //log::info("destroyed special key {}", key);
            cancel();
        }

        bool compareToKey(const std::string& otherKey);

        void setUpdateFunction(geode::Function<UpdateFuture()> task);

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

        void setUpdateCompletedCallback(geode::Function<void(const std::shared_ptr<SpecialKey>&)> callback);
        void setUpdateStartedCallback(geode::Function<void(const std::shared_ptr<SpecialKey>&)> callback);

        void cancel();

        bool doesRefreshWith(const std::string& other);

        void refreshWith(const std::string& other);
        void refreshWith(const std::set<std::string>& others);
};