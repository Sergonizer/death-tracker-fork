#include "SpecialKey.hpp"

SpecialKey::SpecialKey(const std::string& key, const std::string& description){
    this->key = key;
    this->description = description;

    //this->autorelease();
}

bool SpecialKey::compareToKey(const std::string& otherKey){
    return this->key == otherKey;
}

void SpecialKey::setUpdateFunction(const std::function<UpdateTask()>& task){
    this->updateFunction = task;
}

void SpecialKey::updateContent(){
    if (!this->updateFunction.has_value()) return;

    log::info("Starting update for special key {}, {}", this->key, updateOngoing);

    if (!updateOngoing){
        updateOngoing = true;
        if (this->updateStartedCallback)
            this->updateStartedCallback(this->shared_from_this());
    }

    this->updateListener.bind(this, &SpecialKey::onUpdateCompleted);
    this->updateListener.setFilter(this->updateFunction.value()());
}

void SpecialKey::setUpdateCompletedCallback(const std::function<void(const std::shared_ptr<SpecialKey>&)>& callback){
    this->updateCompletedCallback = callback;
}

void SpecialKey::setUpdateStartedCallback(const std::function<void(const std::shared_ptr<SpecialKey>&)>& callback){
    this->updateStartedCallback = callback;
}

void SpecialKey::onUpdateCompleted(UpdateTask::Event* e){
    if (auto* val = e->getValue()){
        if (val == nullptr) return;

        if (val->isErr())
            this->currentContent = fmt::format("Error: {}", val->unwrapErr());
        else
            this->currentContent = val->unwrap();

        if (this->updateCompletedCallback)
            this->updateCompletedCallback(this->shared_from_this());

        updateOngoing = false;
    }
}