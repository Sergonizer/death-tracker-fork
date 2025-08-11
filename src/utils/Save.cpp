#include <utils/Save.hpp>


std::vector<V2LabelLayout> Save::getOldLayout(){
    return Mod::get()->getSavedValue<std::vector<V2LabelLayout>>("Layout");
}

void Save::setOldLayout(const std::vector<V2LabelLayout>& layout){
    Mod::get()->setSavedValue("Layout", layout);
}

std::vector<DTLabelInfo> Save::getLayout(){
    return Mod::get()->getSavedValue<std::vector<DTLabelInfo>>("LabelLayout");
}

void Save::setLayout(const std::vector<DTLabelInfo>& layout){
    Mod::get()->setSavedValue("LabelLayout", layout);
}

ccColor3B Save::getNewBestColor(){
    return Mod::get()->getSavedValue<ccColor3B>("NewBestColor");
}

void Save::setNewBestColor(const ccColor3B& color){
    Mod::get()->setSavedValue("NewBestColor", color);
}

ccColor3B Save::getSessionBestColor(){
    return Mod::get()->getSavedValue<ccColor3B>("SessionBestColor");
}

void Save::setSessionBestColor(const ccColor3B& color){
    Mod::get()->setSavedValue("SessionBestColor", color);
}

bool Save::getExportWOutLabels(){
    return Mod::get()->getSavedValue<bool>("ExportWLabels");
}

void Save::setExportWOutLabels(const bool& b){
    Mod::get()->setSavedValue("ExportWLabels", b);
}

std::string Save::getLastOpenedVersion(){
    return Mod::get()->getSavedValue<std::string>("LastOpenedVersion");
}

void Save::setLastOpenedVersion(const std::string& verion){
    Mod::get()->setSavedValue("LastOpenedVersion", verion);
}

ViewState Save::getLastViewState(){
    return Mod::get()->getSavedValue<ViewState>("LastViewState");
}
void Save::setLastViewState(const ViewState& verion){
    Mod::get()->setSavedValue("LastViewState", verion);
}