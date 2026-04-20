#pragma once

#include <Geode/Geode.hpp>
#include <types/DTTypes.hpp>

using namespace geode::prelude;

class Save {
    public:
        static std::vector<V2LabelLayout> getOldLayout();
        static void setOldLayout(const std::vector<V2LabelLayout>& layout);

        static DTLayoutV3 getLayout();
        static void setLayout(const DTLayoutV3& layout);

        static ccColor3B getNewBestColor();
        static void setNewBestColor(const ccColor3B& color);

        static ccColor3B getSessionBestColor();
        static void setSessionBestColor(const ccColor3B& color);

        static bool getExportWOutLabels();
        static void setExportWOutLabels(const bool& b);

        static bool getDidConvertToV3();
        static void setDidConvertToV3(const bool& b);

        static std::string getLastOpenedVersion();
        static void setLastOpenedVersion(const std::string& verion);

        static stringCustomazations getFrom0Customazations();
        static void setFrom0Customazations(const stringCustomazations& from0Custom);

        static stringCustomazations getRunsCustomazations();
        static void setRunCustomazations(const stringCustomazations& runCustom);

        static stringCustomazations getSessionF0Customazations();
        static void setSessionF0Customazations(const stringCustomazations& sessionF0Custom);

        static stringCustomazations getSessionRunCustomazations();
        static void setSessionRunCustomazations(const stringCustomazations& sessionRunCustom);
        
        static std::vector<DTGraphInfo> getGraphs();
        static void setGraphs(const std::vector<DTGraphInfo>& graphs);

        static bool wasTutorialSeen(const std::string& tutorialID);
        static void setTutorialSeen(const std::string& tutorialID);
};