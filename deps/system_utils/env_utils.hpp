#ifndef ENV_UTILS_HPP
#define ENV_UTILS_HPP


#include <string>

namespace EnvUtils{

inline std::string getEnv(const std::string & key, const std::string & default_value){
   char* env = getenv(key.c_str());
   return ((env!=NULL)?(std::string(env)):default_value);
}

}

#endif // ENV_UTILS_HPP
