//
// Created by 冒家豪 on 2023/9/10.
//

#include "BFS_Solver.h"

#include <exception>
#include <unordered_map>
#include <queue>

/**
 * Find solution to PushBox game use minimal moving steps. Use Breath-first-search method.
 * @param level_data_: level data
 * @param destination_record: record denstination position.
 * destination_record[i][j]=true iff there is a destination at (i,j).
 * @return a vector with elements of value {'u', 'r', 'd', 'l'} represent up, right, down, left moving respectively.
 */
std::vector<char> BFS_Solver::SolveGame(const std::vector<std::vector<GameResource::ObjectName>> &level_data_,
                                        const std::vector<std::vector<bool>> &destination_record) {
    AnalysisLevelData(level_data_, destination_record);
    return BFS(level_data_, destination_record);
}

void BFS_Solver::AnalysisLevelData(const std::vector<std::vector<GameResource::ObjectName>> &level_data,
                                   const std::vector<std::vector<bool>> &destination_record) {
    initial_state_.Reset();
    for (int i = 0; i < level_data.size(); ++i) {
        for (int j = 0; j < level_data[i].size(); ++j) {
            switch (level_data[i][j]) {
                case GameResource::BOX:
                    initial_state_.list_box_pos.emplace(i, j);
                    break;
                case GameResource::PLAYER:
                    initial_state_.player_pos.first = i;
                    initial_state_.player_pos.second = j;
                    break;
                default:
                    break;
            }
        }
    }

    // test the correctness of input date.
    const std::string hint_size_not_equal = "ERROR: The size of level_data and destination_record not match.";
    int num_destination = 0;
    if (destination_record.size() != level_data.size()) {
        throw std::runtime_error(hint_size_not_equal);
    }
    for (int i = 0; i < destination_record.size(); ++i) {
        if (destination_record[i].size() != level_data[i].size()) {
            throw std::runtime_error(hint_size_not_equal);
        }
        for (int j = 0; j < destination_record[i].size(); ++j) {
            if (destination_record[i][j]) {
                ++num_destination;
            }
        }
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
    while (!que.empty() && !find_solution) {
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

    std::vector<char> result;
    if (!find_solution) {
        return result;
    }

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
BFS_Solver::DecodeMoving(PlayerState &end_state, const std::unordered_map<std::string, std::string> &visited) {
    std::vector<char> result;
    PlayerState cur = end_state;
    std::string initial_state_str = initial_state_.ToString();
    std::string cur_str = cur.ToString();
    while (cur_str != initial_state_str) {
        const std::string &previous_state_str = visited.at(cur_str);
        result.push_back(ExtractMoveActionFromStateStr(previous_state_str, cur_str));
        cur_str = previous_state_str;
    }
    std::reverse(result.begin(), result.end());
    return result;
}