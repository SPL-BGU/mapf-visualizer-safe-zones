#include "../include/safe_zones.hpp"

#include <fstream>
#include <iostream>
#include <regex>

// to load graph
static const std::regex r_agent_group =
    std::regex(R"(Safe zone for agent group (\d+)\:)");
static const std::regex r_temporal_graph_start =
    std::regex(R"(Temporal graph start)");
static const std::regex r_temporal_graph_end =
    std::regex(R"(Temporal graph end)");
static const std::regex r_vertex = std::regex(R"(\{([^}]*)\})");
static const std::regex r_range = std::regex(R"(\[(\d+),(\d+)\])");

SafeZones read_temporal_graph(std::ifstream& file)
{
    SafeZones safe_zones;
    std::string line;
    while (getline(file, line)) {
        // for CRLF coding
        if (*(line.end() - 1) == 0x0d) line.pop_back();

        if (std::regex_match(line, r_temporal_graph_end))
            break;  // end of temporal graph

        std::sregex_iterator vertices_begin =
            std::sregex_iterator(line.begin(), line.end(), r_vertex);
        std::sregex_iterator end;
        for (std::sregex_iterator i = vertices_begin; i != end; ++i) {
            SIs safe_intervals;  // Safe intervals for a single vertex
            std::smatch vertex_match = *i;
            std::string content = vertex_match[1];  // contents inside {}

            std::sregex_iterator ranges_begin =
                std::sregex_iterator(content.begin(), content.end(), r_range);
            for (std::sregex_iterator j = ranges_begin; j != end; ++j) {
                std::smatch range_match = *j;
                int range_start = std::stoi(range_match[1]);
                int range_end = std::stoi(range_match[2]);
                safe_intervals.push_back(SafeInterval(range_start, range_end));
            }
            safe_zones.push_back(
                std::move(safe_intervals));  // move to avoid copying
        }
    }
    return safe_zones;
}

SafeZonesManager::SafeZonesManager(const std::string& filename,
                                   int _num_vertices)
    : num_vertices(_num_vertices), agent_groups_safe_zones()
{
    std::ifstream file(filename);
    if (!file) {
        std::cout << "failed to load " << filename << std::endl;
        return;
    }
    std::string line;
    std::smatch results;

    int current_agent_group = -1;
    int last_agent_group = -1;
    while (getline(file, line)) {
        // read each agent group's safe zones
        if (*(line.end() - 1) == 0x0d) line.pop_back();

        if (std::regex_match(line, results, r_agent_group)) {
            current_agent_group = std::stoi(results[1].str());
        }
        if (std::regex_match(line, results, r_temporal_graph_start)) {
            if (current_agent_group == -1) {
                throw std::runtime_error(
                    "Agent group ID must be set before reading safe zones.");
            }
            if (current_agent_group != last_agent_group + 1) {
                throw std::runtime_error(
                    "Agent group IDs must be consecutive. Expected " +
                    std::to_string(last_agent_group + 1) + " but found " +
                    std::to_string(current_agent_group));
            }
            SafeZones safe_zones = read_temporal_graph(file);
            if (safe_zones.size() != num_vertices) {
                throw std::runtime_error(
                    "Safe zones size of agent group " +
                    std::to_string(current_agent_group) +
                    " does not match number of vertices. Expected " +
                    std::to_string(num_vertices) + " but found " +
                    std::to_string(safe_zones.size()));
            }
            agent_groups_safe_zones[current_agent_group] =
                std::move(safe_zones);  // move to avoid copying

            last_agent_group = current_agent_group;
        }
    }
}

int SafeZonesManager::get_safe_agent_group(int vertex_id,
                                            int timestamp) const
{
    if (vertex_id < 0 || vertex_id >= num_vertices) {
        throw std::out_of_range("Vertex ID out of range. Expected 0 to " +
                                std::to_string(num_vertices - 1) +
                                " but got " + std::to_string(vertex_id));
    }
    int agent_group_id = -1;

    for (const auto& [current_agent_group_id, safe_zones] :
         agent_groups_safe_zones) {
        const SIs& safe_intervals = safe_zones[vertex_id];
        for (const SafeInterval& interval : safe_intervals) {
            if (timestamp >= interval.first && timestamp <= interval.second) {
                // Found a safe zone
                if (agent_group_id != -1){
                    std::cerr << "Warning: Multiple safe zones found for vertex "
                              << vertex_id << " at timestamp " << timestamp
                              << ". Ignoring the current agent group (" << current_agent_group_id << ") Returning the first found agent group ID: "
                              << agent_group_id << std::endl;
                }
                else{
                    agent_group_id = current_agent_group_id;
                    break;  // No need to check further intervals for this agent group
                }
            }
        }
    }
    return agent_group_id;
}