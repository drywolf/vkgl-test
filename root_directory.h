#include <filesystem>

const std::string logl_root_str = std::filesystem::current_path().u8string();
const char* logl_root = logl_root_str.c_str();
