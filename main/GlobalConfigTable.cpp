#include "GlobalConfig.h"

const std::map<const char *, std::map<const char *, std::any>> GlobalConfigMap =
    {
        {"watering_config",
         {{"waterDurSec", 60},
          {"waterIntvlSec", 12 * 60 * 60},
          {"waterMode", "cccc"}
          }},

};