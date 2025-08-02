#include "../include/param.hpp"

static std::vector<ofColor> agent_groups;

ofColor Color::get_agent_group_color(int group_id)
{
    if (group_id < 0 || group_id >= agent_groups.size()) {
        throw std::out_of_range("Group ID out of range");
    }
    return agent_groups[group_id];
}

void Color::init_agent_groups(int num_groups)
{
    agent_groups.clear();
    agent_groups.reserve(num_groups);
    for (int i = 1; i <= num_groups; ++i) {
        agent_groups.emplace_back(
            (i * 50) % 256,
            (i * 80) % 256,
            (i * 30) % 256
        );
    }
}