//
// Created by 冒家豪 on 2023/9/10.
//

#ifndef DEMO_IMGUI_BFS_SOLVER_H
#define DEMO_IMGUI_BFS_SOLVER_H

#include "GameResource.h"

#include <set>
#include <iostream>
#include <atomic>
#include <regex>

class BFS_Solver {
public:
    std::atomic<bool> abrupt_flag{false};
    std::atomic<bool> solver_done_flag{false};
    bool no_solution_flag = false;

    std::vector<char> SolveGame(const GameResource::GameLevelData &level_data_,
                                const std::vector<std::vector<bool>> &destination_record);

    struct PlayerState {
        std::pair<int, int> player_pos;
        std::set<std::pair<int, int>> list_box_pos;

        std::string ToString() const {
            std::string res;
            res = std::to_string(player_pos.first) + "p" + std::to_string(player_pos.second);
            for (const auto &box_pos: list_box_pos) {
                res += "b" + std::to_string(box_pos.first) + "b" + std::to_string(box_pos.second);
            }
            return res;
        }

        static PlayerState Decode(const std::string &str) {
            PlayerState res;
            std::smatch match_res;
            std::regex_search(str, match_res, player_reg);
            res.player_pos.first = std::stoi(match_res[1]);
            res.player_pos.second = std::stoi(match_res[2]);
            std::string tmp_str = match_res.suffix();
            while (std::regex_search(tmp_str, match_res, box_pos_reg)) {
                res.list_box_pos.emplace(std::stoi(match_res[1]), std::stoi(match_res[2]));
                tmp_str = match_res.suffix();
            }
            return res;
        }

        void Reset() {
            list_box_pos.clear();
        }

        static const std::regex player_reg;
        static const std::regex box_pos_reg;
    };

private:
    void AnalysisLevelData(const GameResource::GameLevelData &level_data,
                           const std::vector<std::vector<bool>> &destination_record);

    std::vector<char> BFS(const std::vector<std::vector<GameResource::ObjectName>> &level_data,
                          const std::vector<std::vector<bool>> &destination_record);

    std::vector<char> BFS_AStar(const std::vector<std::vector<GameResource::ObjectName>> &level_data,
                                const std::vector<std::vector<bool>> &destination_record);

    std::vector<char> DijkstraMethod(const std::vector<std::vector<GameResource::ObjectName>> &level_data,
                                     const std::vector<std::vector<bool>> &destination_record);

    int AStarEstimateDistanceToEnd(const PlayerState &state) const;

    static bool CheckMoving(const PlayerState &current_state, const int move_dx[2],
                            const std::vector<std::vector<GameResource::ObjectName>> &level_data,
                            PlayerState &moved_state);

    static bool CheckArrived(const PlayerState &current_state, const std::vector<std::vector<bool>> &list_destination);

    std::vector<char>
    DecodeMoving(PlayerState &end_state, const std::unordered_map<std::string, std::string> &previous);

    PlayerState initial_state_;
    std::set<std::pair<int, int>> destination_pos;
    int map_n_rows_, map_n_cols_;

    static constexpr int NUM_DIRECTIONS = 4;
    static const int move_dx[NUM_DIRECTIONS][2];

    struct DijkstraDistanceMap {
        int *dist_map = nullptr;

        DijkstraDistanceMap(int map_n_rows, int map_n_cols) : nrows_(map_n_rows), ncols_(map_n_cols) {
            dist_map = new int[map_n_rows * map_n_cols];
        }

        void GenerateMapDistance(const std::vector<std::vector<GameResource::ObjectName>> &level_data,
                                 const PlayerState &player_state);

        int operator()(int i, int j) {
            return dist_map[i * ncols_ + j];
        }

        ~DijkstraDistanceMap() {
            delete[]dist_map;
            dist_map = nullptr;
        }

    private:
        int nrows_, ncols_;
    };
};

#endif //DEMO_IMGUI_BFS_SOLVER_H
