//
// Created by 冒家豪 on 2023/9/10.
//

#include "BFS_Solver.h"

#include <exception>
#include <unordered_map>
#include <unordered_set>
#include <queue>

#include <iostream>

/**
 * Find solution to PushBox game use minimal moving steps. Use Breath-first-search method.
 * @param level_data_: level data
 * @param destination_record: record denstination position.
 * destination_record[i][j]=true iff there is a destination at (i,j).
 * @return a vector with elements of value {'u', 'r', 'd', 'l'} represent up, right, down, left moving respectively.
 */
std::vector<char> BFS_Solver::SolveGame(const GameResource::GameLevelData &level_data_,
                                        const std::vector<std::vector<bool>> &destination_record) {
    AnalysisLevelData(level_data_, destination_record);
//     return BFS_AStar(level_data_, destination_record);
    return BFS(level_data_.game_wall_map, destination_record);
}

void BFS_Solver::AnalysisLevelData(const GameResource::GameLevelData &level_data,
                                   const std::vector<std::vector<bool>> &destination_record) {
    initial_state_.Reset();
    for (auto &box_pos: level_data.box_positions) {
        initial_state_.list_box_pos.emplace(box_pos.y, box_pos.x);
    }
    initial_state_.player_pos.first = level_data.player_position.y;
    initial_state_.player_pos.second = level_data.player_position.x;

    // test the correctness of input date.
    int num_destination = level_data.destination_positions.size();
    for (auto &dest_pos: level_data.destination_positions) {
        destination_pos.emplace(dest_pos.y, dest_pos.x);
    }

    if (num_destination != initial_state_.list_box_pos.size()) {
        throw std::runtime_error("The number of box is not equal to number of destinations.");
    }
}

/**
 * Check moving is avalible
 * @param current_state
 * @param move_dx
 * @param level_data
 * @param[out] moved_state: if can move, the moved state will return by reference.
 * @return
 */
bool BFS_Solver::CheckMoving(const PlayerState &current_state, const int move_dx[2],
                             const std::vector<std::vector<GameResource::ObjectName>> &level_data,
                             PlayerState &moved_state) {
    std::pair<int, int> moved_player_pos{current_state.player_pos.first + move_dx[0],
                                         current_state.player_pos.second + move_dx[1]};
    if (level_data[moved_player_pos.first][moved_player_pos.second] == GameResource::WALL) {
        return false;
    }

    moved_state.list_box_pos = current_state.list_box_pos;
    auto itr_box_pos = moved_state.list_box_pos.find(moved_player_pos);
    if (itr_box_pos != moved_state.list_box_pos.end()) {
        std::pair<int, int> moved_box(moved_player_pos.first + move_dx[0], moved_player_pos.second + move_dx[1]);
        if (level_data[itr_box_pos->first][itr_box_pos->second] == GameResource::WALL ||
            moved_state.list_box_pos.find(moved_box) != moved_state.list_box_pos.end()) {
            return false;
        }
        moved_state.list_box_pos.erase(itr_box_pos);
        moved_state.list_box_pos.emplace(std::move(moved_box));
    }
    moved_state.player_pos = std::move(moved_player_pos);
    return true;
}

bool BFS_Solver::CheckArrived(const PlayerState &current_state,
                              const std::vector<std::vector<bool>> &list_destination) {
    return std::all_of(current_state.list_box_pos.begin(), current_state.list_box_pos.end(),
                       [&](const std::pair<int, int> &box_pos) {
                           return list_destination[box_pos.first][box_pos.second];
                       });
}

std::vector<char> BFS_Solver::BFS(const std::vector<std::vector<GameResource::ObjectName>> &level_data,
                                  const std::vector<std::vector<bool>> &destination_record) {
    PlayerState end_state;
    const int NUM_DIRECTIONS = 4;
    const char move_ch[] = "urdl";
    const int move_dx[][2] = {{-1, 0},
                              {0,  1},
                              {1,  0},
                              {0,  -1}};
    std::unordered_map<std::string, std::string> visited; // map from state string to previous state string before moving.
    std::queue<PlayerState> que;
    que.push(initial_state_);
    visited[initial_state_.ToString()] = "0";
    bool find_solution = false;
    int step = 0;
    while (!que.empty() && !find_solution) {
        int que_sz = que.size();
        for (int que_index = 0; que_index < que_sz; ++que_index) {
            auto cur = que.front();
            std::string cur_str = cur.ToString();
            que.pop();
            for (int i = 0; i < NUM_DIRECTIONS; ++i) {
                PlayerState moved_state;
                if (!CheckMoving(cur, move_dx[i], level_data, moved_state)) {
                    continue;
                }
                std::string moved_state_string = moved_state.ToString();
                if (visited.find(moved_state_string) != visited.end()) {
                    continue;
                }
                visited[moved_state_string] = cur_str;

                if (CheckArrived(moved_state, destination_record)) {
                    end_state = moved_state;
                    find_solution = true;
                    break;
                }

                que.push(moved_state);
            }
        }
        ++step;
        std::cout << "searched step: " << step << ", visited states at this step: " << que_sz << std::endl;
    }

    std::vector<char> result;
    if (!find_solution) {
        return result;
    }
    std::cout << "number of visited states: " << visited.size() << "\n";
    result = DecodeMoving(end_state, visited);
    return result;
}

std::pair<int, int> StateStrToPlayerPos(const std::string &state_str) {
    size_t pos_p_ch = state_str.find('p');
    size_t pos_b_ch = state_str.find('b', pos_p_ch + 1);
    std::pair<int, int> pos_player;
    pos_player.first = std::stoi(state_str.substr(0, pos_p_ch));
    pos_player.second = std::stoi(state_str.substr(pos_p_ch + 1, pos_b_ch - pos_p_ch));
    return pos_player;
}

char ExtractMoveActionFromStateStr(const std::string &previous_str, const std::string &current_str) {
    auto pre_pos = StateStrToPlayerPos(previous_str);
    auto cur_pos = StateStrToPlayerPos(current_str);
    if (cur_pos.second == pre_pos.second - 1) {
        return 'l';
    }
    if (cur_pos.second == pre_pos.second + 1) {
        return 'r';
    }
    if (cur_pos.first == pre_pos.first - 1) {
        return 'u';
    }
    if (cur_pos.first == pre_pos.first + 1) {
        return 'd';
    }
    throw std::runtime_error("Error in State String, cannot extract moving action");
}

std::vector<char>
BFS_Solver::DecodeMoving(PlayerState &end_state, const std::unordered_map<std::string, std::string> &previous) {
    std::vector<char> result;
    PlayerState cur = end_state;
    std::string initial_state_str = initial_state_.ToString();
    std::string cur_str = cur.ToString();
    while (cur_str != initial_state_str) {
        const std::string &previous_state_str = previous.at(cur_str);
        result.push_back(ExtractMoveActionFromStateStr(previous_state_str, cur_str));
        cur_str = previous_state_str;
    }
    std::reverse(result.begin(), result.end());
    return result;
}

int BFS_Solver::AStarEstimateDistanceToEnd(const PlayerState &state) const {
    int dist = 0;
    auto itr1 = state.list_box_pos.begin();
    auto itr2 = destination_pos.begin();
    while (itr2 != destination_pos.end()) {
        dist += std::abs(itr2->first - itr1->first) + std::abs(itr2->second - itr1->second);
        ++itr1;
        ++itr2;
    }
    return dist;
}

std::vector<char> BFS_Solver::BFS_AStar(const std::vector<std::vector<GameResource::ObjectName>> &level_data,
                                        const std::vector<std::vector<bool>> &destination_record) {
    PlayerState end_state;
    const int NUM_DIRECTIONS = 4;
    const char move_ch[] = "urdl";
    const int move_dx[][2] = {{-1, 0},
                              {0,  1},
                              {1,  0},
                              {0,  -1}};
    std::unordered_map<std::string, std::string> previous; // map from state string to previous state string before moving.
    std::unordered_map<std::string, int> distance;
    std::unordered_set<std::string> known;
    auto comp_greater = [&](const PlayerState &x, const PlayerState &y) -> bool {
        return distance.at(x.ToString()) + AStarEstimateDistanceToEnd(x) >
               distance.at(y.ToString()) + AStarEstimateDistanceToEnd(y);
    };
    std::priority_queue<PlayerState, std::vector<PlayerState>, decltype(comp_greater)> heap(comp_greater);
    heap.push(initial_state_);
    previous[initial_state_.ToString()] = "0";
    distance[initial_state_.ToString()] = 0;

    bool find_solution = false;
    int step = 0;
    while (!heap.empty() && !find_solution) {
        int sz = heap.size();
        for (int ind = 0; ind < sz; ++ind) {
            auto cur = heap.top();
            auto cur_str = cur.ToString();
            heap.pop();
            while (!heap.empty() && known.find(cur_str) != known.end()) {
                cur = heap.top();
                cur_str = cur.ToString();
                heap.pop();
            }
            if (known.find(cur_str) != known.end()) {
                break;
            }
            known.insert(cur_str);

            for (int i = 0; i < NUM_DIRECTIONS; ++i) {
                PlayerState moved_state;
                if (!CheckMoving(cur, move_dx[i], level_data, moved_state)) {
                    continue;
                }

                if (CheckArrived(moved_state, destination_record)) {
                    end_state = moved_state;
                    previous[end_state.ToString()] = cur_str;
                    find_solution = true;
                    break;
                }

                std::string moved_state_string = moved_state.ToString();
                if (known.find(moved_state_string) != known.end()) {
                    continue;
                }

                int estimate_next_end_dist = AStarEstimateDistanceToEnd(moved_state);
                int next_dist = distance[cur_str] + 1 + estimate_next_end_dist;
                if (distance.find(moved_state_string) == distance.end() ||
                    next_dist < distance[moved_state_string] + estimate_next_end_dist) {
                    distance[moved_state_string] = distance[cur_str] + 1;
                    heap.push(moved_state);
                    previous[moved_state_string] = cur_str;
                }
            }
        }
        ++step;
        std::cout << "step: " << step << ", num of visited states at this steps: " << sz << std::endl;
    }

    std::vector<char> result;
    if (!find_solution) {
        return result;
    }
    std::cout << "number of visited states: " << distance.size() << "\n";
    result = DecodeMoving(end_state, previous);
    return result;
}