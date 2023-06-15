#include "GlobalConfig.h"

const std::map<const char *,
               std::map<const char *, std::variant<std::string, int, bool>>>
    GlobalConfigMap =
        {
            {"watering_config",
             {{"waterDurSec", 60},
              {"waterIntvlSec", 12 * 60 * 60},
              {"waterMode", std::string("It's OK")},
              {"waterNm", std::string("I am water")}}},

};