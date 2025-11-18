#include <utils/Save.hpp>


std::vector<V2LabelLayout> Save::getOldLayout(){
    return Mod::get()->getSavedValue<std::vector<V2LabelLayout>>("Layout");
}

void Save::setOldLayout(const std::vector<V2LabelLayout>& layout){
    Mod::get()->setSavedValue("Layout", layout);
}

DTLayoutV3 Save::getLayout(){
    return Mod::get()->getSavedValue<DTLayoutV3>("DTLayoutV3");
}

void Save::setLayout(const DTLayoutV3& layout){
    Mod::get()->setSavedValue("DTLayoutV3", layout);
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

stringCustomazations Save::getFrom0Customazations(){
    return Mod::get()->getSavedValue<stringCustomazations>("F0Zustom");
}
void Save::setFrom0Customazations(const stringCustomazations& from0Custom){
    Mod::get()->setSavedValue("F0Zustom", from0Custom);
}

stringCustomazations Save::getRunsCustomazations(){
    return Mod::get()->getSavedValue<stringCustomazations>("RunCustom");
}
void Save::setRunCustomazations(const stringCustomazations& runCustom){
    Mod::get()->setSavedValue("RunCustom", runCustom);
}

stringCustomazations Save::getSessionF0Customazations(){
    return Mod::get()->getSavedValue<stringCustomazations>("SessionF0Custom");
}
void Save::setSessionF0Customazations(const stringCustomazations& sessionF0Custom){
    Mod::get()->setSavedValue("SessionF0Custom", sessionF0Custom);
}

stringCustomazations Save::getSessionRunCustomazations(){
    return Mod::get()->getSavedValue<stringCustomazations>("SessionRunCustom");
}
void Save::setSessionRunCustomazations(const stringCustomazations& sessionRunCustom){
    Mod::get()->setSavedValue("SessionRunCustom", sessionRunCustom);
}