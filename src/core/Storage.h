#pragma once
#include <string>

namespace Storage {

void        save(const std::string& key, const std::string& value);
std::string load(const std::string& key);
bool        exists(const std::string& key);
void        remove(const std::string& key);

} // namespace Storage
