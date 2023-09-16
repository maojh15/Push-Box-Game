//
// Created by 冒家豪 on 2023/9/10.
//

#include "BFS_Solver.h"

#include <exception>
#include <unordered_map>
#include <unordered_set>
#include <queue>

#include <iostream>

const std::regex BFS_Solver::PlayerState::player_reg = std::regex("(\\d*)p(\\d*)");
const std::regex BFS_Solver::PlayerState::box_pos_reg = std::regex("b(\\d*)b(\\d*)");
const int BFS_Solver::move_dx[BFS_Solver::NUM_DIRECTIONS][2] = {{-1, 0},
                                                                {0,  1},
                                                                {1,  0},
                                                                {0,  -1}};

/**
 * Find solution to PushBox game use minimal moving steps. Use Breath-first-search method.
 * @param level_data_: level data
 * @param destination_record: record denstination position.
 * destination_record[i][j]=true iff there is a destination at (i,j).
 * @return a vector with elements of value {'u', 'r', 'd', 'l'} represent up, right, down, left moving respectively.
 */
std::vector<char> BFS_Solver::SolveGame(const GameResource::GameLevelData &level_data_,
                                        const std::vector<std::vector<bool>> &destination_record) {
    abrupt_flag = false;
    no_solution_flag = false;
    AnalysisLevelData(level_data_, destination_record);
    std::vector<char> res;
    if (!CheckArrived(initial_state_, destination_record)) {
        res = DijkstraMethod(level_data_.game_wall_map, destination_record);
//        res = BFS_AStar(level_data_, destination_record);
//        res = BFS(level_data_.game_wall_map, destination_record);
    }
    solver_done_flag = true;
    return res;
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

    map_n_rows_ = level_data.game_wall_map.size();
    map_n_cols_ = 0;
    for (auto &elem: level_data.game_wall_map) {
        map_n_cols_ = std::max<int>(map_n_cols_, elem.size());
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
    std::unordered_map<std::string, std::string> visited; // map from state string to previous state string before moving.
    std::queue<PlayerState> que;
    que.push(initial_state_);
    visited[initial_state_.ToString()] = "0";
    bool find_solution = false;
    int step = 0;
    while (!que.empty() && !find_solution) {
        int que_sz = que.size();
        for (int que_index = 0; que_index < que_sz; ++que_index) {
            if (abrupt_flag) {
                return {};
            }
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
        no_solution_flag = true;
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
        no_solution_flag = true;
        return result;
    }
    std::cout << "number of visited states: " << distance.size() << "\n";
    result = DecodeMoving(end_state, previous);
    return result;
}

void BFS_Solver::DijkstraDistanceMap::GenerateMapDistance(
        const std::vector<std::vector<GameResource::ObjectName>> &level_data,
        const PlayerState &player_state) {
    std::fill(&dist_map[0], &dist_map[nrows_ * ncols_], -1);
    dist_map[player_state.player_pos.first * ncols_ + player_state.player_pos.second] = 0;

    std::queue<std::pair<int, int>> que;
    que.push(player_state.player_pos);
    while (!que.empty()) {
        auto cur = que.front();
        que.pop();
        int cur_dist = dist_map[cur.first * ncols_ + cur.second];
        for (int i = 0; i < NUM_DIRECTIONS; ++i) {
            std::pair<int, int> moved(cur.first + move_dx[i][0], cur.second + move_dx[i][1]);
            int moved_index = moved.first * ncols_ + moved.second;
            if (dist_map[moved_index] >= 0 ||
                level_data[moved.first][moved.second] == GameResource::WALL ||
                player_state.list_box_pos.find(moved) != player_state.list_box_pos.end()) {
                continue;
            }
            dist_map[moved_index] = cur_dist + 1;
            que.push(moved);
        }
    }
}

namespace {
    struct RecordMoveAction {
        RecordMoveAction() = default;

        RecordMoveAction(const std::pair<int, int> &player_from, const std::pair<int, int> &box_pos_before_move,
                         const std::pair<int, int> &box_pos_after_move)
                : player_from(player_from), box_pos_before_move(box_pos_before_move),
                  box_pos_after_move(box_pos_after_move) {}

        std::pair<int, int> player_from, box_pos_before_move, box_pos_after_move;
    };

    char MoveDxToDirectionChar(const std::pair<int, int> &move_dx) {
        if (move_dx.first == 0) {
            return move_dx.second == 1 ? 'r' : 'l';
        }
        return move_dx.first == 1 ? 'd' : 'u';
    };

    struct AStarPathSearcher {
        int n_rows, n_cols;
        int *map_record = nullptr;
        bool *known = nullptr;
        int *previous = nullptr;
        static const int direction[4][2];

        AStarPathSearcher(int n_rows, int n_cols) : n_rows(n_rows), n_cols(n_cols) {
            map_record = new int[n_rows * n_cols];
            known = new bool[n_rows * n_cols];
            previous = new int[n_rows * n_cols];
        }

        ~AStarPathSearcher() {
            delete[]map_record;
            map_record = nullptr;
            delete[]known;
            known = nullptr;
            delete[]previous;
            previous = nullptr;
        }

/**
 * Use A*-algorithm to search path. Notice the returned path is in inverse order (i.e. the path returned is
 * start from end_pos to start_pos.)
 * @param level_data
 * @param list_box_pos
 * @param start_pos
 * @param end_pos
 * @return
 */
        std::vector<char> AStarSearchPathInverse(const std::vector<std::vector<GameResource::ObjectName>> &level_data,
                                                 const std::set<std::pair<int, int>> &list_box_pos,
                                                 const std::pair<int, int> &start_pos,
                                                 const std::pair<int, int> &end_pos) {
            if (start_pos == end_pos) {
                return {};
            }

            std::fill(&map_record[0], &map_record[n_rows * n_cols], std::numeric_limits<int>::max());
            std::fill(&known[0], &known[n_rows * n_cols], false);
            std::fill(&previous[0], &previous[n_rows * n_cols], -1);
            for (int i = 0; i < level_data.size(); ++i) {
                for (int j = 0; j < level_data[i].size(); ++j) {
                    if (level_data[i][j] == GameResource::WALL) {
                        map_record[i * n_cols + j] = -1;
                    }
                }
            }
            for (auto &box_pos: list_box_pos) {
                map_record[box_pos.first * n_cols + box_pos.second] = -1;
            }
            map_record[start_pos.first * n_cols + start_pos.second] = 0;

            auto estimate_dist_to_end = [&end_pos](const std::pair<int, int> &cur_pos) {
                return std::abs(cur_pos.first - end_pos.first) + std::abs(cur_pos.second - end_pos.second);
            };
            auto comp_dist = [&](const std::pair<int, int> &a, const std::pair<int, int> &b) {
                return map_record[a.first * n_cols + a.second] + estimate_dist_to_end(a) >
                       map_record[b.first * n_cols + b.second] + estimate_dist_to_end(b);
            };
            std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, decltype(comp_dist)> heap(
                    comp_dist);

            int end_pos_index = end_pos.first * n_cols + end_pos.second;

            heap.push(start_pos);
            bool is_arrived = false;
            while (!is_arrived && !heap.empty()) {
                auto cur = heap.top();
                heap.pop();
                int cur_index = cur.first * n_cols + cur.second;
                while (!heap.empty() && known[cur_index]) {
                    cur = heap.top();
                    heap.pop();
                    cur_index = cur.first * n_cols + cur.second;
                }
                if (known[cur_index]) {
                    throw std::runtime_error("No path exist from start_point to end point");
                }
                known[cur_index] = true;
                int cur_dist = map_record[cur_index];

                for (auto &dx: direction) {
                    std::pair<int, int> moved(cur.first + dx[0], cur.second + dx[1]);
                    int moved_index = moved.first * n_cols + moved.second;
                    if (map_record[moved_index] < 0 || known[moved_index]) {
                        continue;
                    }
                    int new_dist = cur_dist + 1;
                    if (new_dist < map_record[moved_index]) {
                        map_record[moved_index] = new_dist;
                        heap.push(moved);
                        previous[moved_index] = cur_index;
                        if (moved_index == end_pos_index) {
                            is_arrived = true;
                            break;
                        }
                    }
                }
            }

            if (!is_arrived) {
                throw std::runtime_error("No path exist from start_point to end point");
            }
            std::vector<char> result;
            int cur_index = end_pos_index;
            int start_index = start_pos.first * n_cols + start_pos.second;
            std::pair<int, int> cur_pos = end_pos;
            while (cur_index != start_index) {
                int pre_index = previous[cur_index];
                std::pair<int, int> pre_pos(pre_index / n_cols, pre_index % n_cols);
                std::pair<int, int> pos_change(cur_pos.first - pre_pos.first, cur_pos.second - pre_pos.second);
                result.emplace_back(MoveDxToDirectionChar(pos_change));
                cur_index = pre_index;
                cur_pos = pre_pos;
            }
            return result;
        }
    };

    const int AStarPathSearcher::direction[4][2] = {{0,  1},
                                                    {0,  -1},
                                                    {1,  0},
                                                    {-1, 0}};
}

std::vector<char>
RetrieveDijkstraMovingSteps(const std::vector<std::vector<GameResource::ObjectName>> &level_data,
                            const std::unordered_map<std::string, RecordMoveAction> &previous,
                            const BFS_Solver::PlayerState &start_state, const BFS_Solver::PlayerState &end_state,
                            int map_n_rows, int map_n_cols) {
    AStarPathSearcher path_searcher(map_n_rows, map_n_cols);
    std::vector<char> result;
    BFS_Solver::PlayerState cur_state = end_state;
    auto cur_str = cur_state.ToString();
    auto start_str = start_state.ToString();
    while (cur_str != start_str) {
        const auto &record_action = previous.at(cur_str);
        std::pair<int, int> pos_change(record_action.box_pos_after_move.first - record_action.box_pos_before_move.first,
                                       record_action.box_pos_after_move.second -
                                       record_action.box_pos_before_move.second);
        result.emplace_back(MoveDxToDirectionChar(pos_change));
        std::pair<int, int> player_end(record_action.box_pos_before_move.first - pos_change.first,
                                       record_action.box_pos_before_move.second - pos_change.second);
        cur_state.list_box_pos.erase(record_action.box_pos_after_move);
        cur_state.list_box_pos.emplace(record_action.box_pos_before_move);
        auto player_move_inverse_path =
                path_searcher.AStarSearchPathInverse(level_data, cur_state.list_box_pos, record_action.player_from,
                                                     player_end);
        for (auto &ch: player_move_inverse_path) {
            result.emplace_back(ch);
        }
        cur_state.player_pos = record_action.player_from;
        cur_str = cur_state.ToString();
    }
    std::reverse(result.begin(), result.end());
    return result;
}

std::vector<char> BFS_Solver::DijkstraMethod(const std::vector<std::vector<GameResource::ObjectName>> &level_data,
                                             const std::vector<std::vector<bool>> &destination_record) {
    std::unordered_map<std::string, int> distance;
    std::unordered_set<std::string> known;
    std::unordered_map<std::string, RecordMoveAction> previous;
    auto comp_dist = [&](const std::string &s1, const std::string &s2) -> bool {
        auto itr1 = distance.find(s1);
        if (itr1 == distance.end()) {
            return true;
        }
        auto itr2 = distance.find(s2);
        if (itr2 == distance.end()) {
            return false;
        }
        return itr1->second > itr2->second;
    };
    std::priority_queue<std::string, std::vector<std::string>, decltype(comp_dist)> heap(comp_dist);

    // push initial state.
    auto start_str = initial_state_.ToString();
    heap.push(start_str);
    distance[start_str] = 0;

    PlayerState end_state;
    DijkstraDistanceMap dist_map(map_n_rows_, map_n_cols_);

    bool arrive_end_point = false;
    // Dijkstra algorithm
    while (!arrive_end_point && !heap.empty()) {
        if (abrupt_flag) {
            return {};
        }
        auto cur_str = heap.top();
        heap.pop();
        while (!heap.empty() && known.find(cur_str) != known.end()) {
            cur_str = heap.top();
            heap.pop();
        }
        if (known.find(cur_str) != known.end()) {
            return {};
        }
        known.insert(cur_str);

        auto cur = PlayerState::Decode(cur_str);
        dist_map.GenerateMapDistance(level_data, cur);
        // visit each box.
        for (auto &box_pos: cur.list_box_pos) {
            for (int i = 0; i < NUM_DIRECTIONS; ++i) {
                std::pair<int, int> pos_player_in_front_box(box_pos.first - move_dx[i][0],
                                                            box_pos.second - move_dx[i][1]);
                if (dist_map(pos_player_in_front_box.first, pos_player_in_front_box.second) < 0) {
                    continue;
                }
                std::pair<int, int> moved_box_pos(box_pos.first + move_dx[i][0],
                                                  box_pos.second + move_dx[i][1]);
                if (level_data[moved_box_pos.first][moved_box_pos.second] == GameResource::WALL ||
                    cur.list_box_pos.find(moved_box_pos) != cur.list_box_pos.end()) {
                    continue;
                }
                int next_dist =
                        distance[cur_str] + dist_map(pos_player_in_front_box.first, pos_player_in_front_box.second) + 1;
                PlayerState moved_state;
                moved_state.player_pos = box_pos;
                moved_state.list_box_pos = cur.list_box_pos;
                moved_state.list_box_pos.erase(box_pos);
                moved_state.list_box_pos.insert(moved_box_pos);
                auto moved_state_str = moved_state.ToString();
                if (known.find(moved_state_str) != known.end() ||
                    (distance.find(moved_state_str) != distance.end() && distance[moved_state_str] <= next_dist)) {
                    continue;
                }
                distance[moved_state_str] = next_dist;
                heap.push(moved_state_str);
                previous.emplace(moved_state_str,
                                 RecordMoveAction(cur.player_pos, box_pos, moved_box_pos));
                if (CheckArrived(moved_state, destination_record)) {
                    arrive_end_point = true;
                    end_state = std::move(moved_state);
                    break;
                }
            }
            if (arrive_end_point) {
                break;
            }
        }
    }

    std::cout << "Number of visited states: " << previous.size() << std::endl;

    if (!arrive_end_point) {
        no_solution_flag = true;
        return {};
    }

    // Retrieve player action.
    auto result = RetrieveDijkstraMovingSteps(level_data, previous, initial_state_, end_state, map_n_rows_,
                                              map_n_cols_);
    return result;
}