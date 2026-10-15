#include "BattleLog.h"

void BattleLog::add(const std::string& line) {
    lines_.push_back(line);
    if (lines_.size() > kMaxLines) lines_.erase(lines_.begin());
}

void BattleLog::addEvent(const BattleEvent& event) { pendingEvents_.push_back(event); }

std::vector<std::string> BattleLog::lastLines(std::size_t count) const {
    if (lines_.size() <= count) return lines_;
    return std::vector<std::string>(lines_.end() - static_cast<long>(count), lines_.end());
}

std::vector<BattleEvent> BattleLog::takeEvents() {
    std::vector<BattleEvent> out;
    out.swap(pendingEvents_);
    return out;
}
