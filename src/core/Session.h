#pragma once
#include "Types.h"
#include <string>

namespace Session {

std::string toJson(const DecisionSession& s);
DecisionSession fromJson(const std::string& json);

} // namespace Session
