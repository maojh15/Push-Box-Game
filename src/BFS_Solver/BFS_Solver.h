//
// Created by 冒家豪 on 2023/9/10.
//

#ifndef DEMO_IMGUI_BFS_SOLVER_H
#define DEMO_IMGUI_BFS_SOLVER_H

#include "GameResource.h"

#include <set>
#include <iostream>
#include <atomic>

class BFS_Solver {
public:
    std::atomic<bool> abrupt_flag{false};
    std::atomic<bool> solver_done_flag{false};

    std::vector<char> SolveGame(const GameResource::GameLevelData &level_data_,
                                const std::vector<std::vector<bool>> &destination_record);

private:
    void AnalysisLevelData(const GameResource::GameLevelData &level_data,
                           const std::vector<std::vector<bool>> &destination_record);

    std::vector<char> BFS(const std::vector<std::vector<GameResource::ObjectName>> &level_data,
                          const std::vector<std::vector<bool>> &destination_record);

    std::vector<char> BFS_AStar(const std::vector<std::vector<GameResource::ObjectName>> &level_data,
                                const std::vector<std::vector<bool>> &destination_record);

    struct PlayerState {
        std::pair<int, int> player_pos;
        std::set<std::pair<int, int>> list_box_pos;
        std::pair<int, int> pre_player_pos;

        std::string ToString() const {
            std::string res;
            res = std::to_string(player_pos.first) + "p" + std::to_string(player_pos.second);
            for (const auto &box_pos: list_box_pos) {
                res += "b" + std::to_string(box_pos.first) + "b" + std::to_string(box_pos.second);
            }
            return res;
        }

        void Reset() {
            list_box_pos.clear();
        }
    };

    int AStarEstimateDistanceToEnd(const PlayerState &state) const;

    static bool CheckMoving(const PlayerState &current_state, const int move_dx[2],
                            const std::vector<std::vector<GameResource::ObjectName>> &level_data,
                            PlayerState &moved_state);

    static bool CheckArrived(const PlayerState &current_state, const std::vector<std::vector<bool>> &list_destination);

    std::vector<char>
    DecodeMoving(PlayerState &end_state, const std::unordered_map<std::string, std::string> &previous);

    PlayerState initial_state_;
    std::set<std::pair<int, int>> destination_pos;
};


#endif //DEMO_IMGUI_BFS_SOLVER_H
