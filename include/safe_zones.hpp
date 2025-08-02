/**
 * @file safe_zones.hpp
 * @author Rotem Lev Lehman (levlerot@post.bgu.ac.il)
 * @brief This file defines the SafeZones class, which manages safe zones in a graph.
 * @version 0.1
 * @date 2025-07-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include <vector>
#include <string>
#include <unordered_map>

typedef std::pair<int, int> SafeInterval;  // [start, end]

typedef std::vector<SafeInterval> SIs;  // Safe intervals for a single vertex

typedef std::vector<SIs> SafeZones;  // Safe zones for the entire graph for a single agent group

class SafeZonesManager {
private:
    std::unordered_map<int, SafeZones> agent_groups_safe_zones;  // Maps agent group ID to its safe zones
    int num_vertices;  // Number of vertices in the graph
public:
    SafeZonesManager(const std::string& filename, int _num_vertices);

    int get_safe_agent_group(int vertex_id, int timestamp) const;
    int get_num_agent_groups() const {
        return agent_groups_safe_zones.size();
    }
};
