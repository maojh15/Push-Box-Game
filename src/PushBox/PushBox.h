//
// Created by 冒家豪 on 2023/8/27.
//

#ifndef DEMO_IMGUI_PUSHBOX_H
#define DEMO_IMGUI_PUSHBOX_H

#include "GameResource.h"
#include "SDL.h"
#include "BFS_Solver.h"
#include "LevelEditor.h"

#define GAME_VERSION "version 1.1.0"

#include <thread>
#include <future>

class PushBox {
public:
    enum GameState {
        START, PLAYING, PREWIN, WIN, END,
        LEVEL_EDITOR
    };

    PushBox() {
        game_state_ = START;
        level_editor_.SetGameResourcePtr(game_resource_);
    }

    void Render(float game_area_height = 300, float game_area_width = 300);

    GameResource GetGameResource() { return game_resource_; }

    bool ProcessInput(SDL_Event &event);

    void CheckGameState();

    void InitializeGame();

    void RevokeOneStep() {
        game_record_.RecoverRecord(*this);
    }

    void BFSSolveGame() {
        static std::future<std::vector<char>> future;
        static std::chrono::steady_clock::time_point time1;
        if (!solver_working) {
            game_solver.solver_done_flag = false;
            game_solver.abrupt_flag = false;
            time1 = std::chrono::steady_clock::now();
            future = std::async(&BFS_Solver::SolveGame, &game_solver, *game_level_ptr_, destination_record_);
            solver_working = true;
        } else if (game_solver.solver_done_flag) {
            auto time2 = std::chrono::steady_clock::now();
            solver_working = false;
            bfs_result = future.get();

            int count_step = 0;
            std::cout << "======BFS Solution==========\n";
            std::string output_result;
            for (char ch: bfs_result) {
                output_result.push_back(ch);
                output_result.push_back(' ');
                ++count_step;
                if (count_step % 10 == 0) {
                    output_result.push_back('\n');
                }
            }
            std::cout << output_result;
            std::cout << "\ntotal steps: " << bfs_result.size() << std::endl;
            std::cout << "cost time: " << std::chrono::duration<double, std::milli>(time2 - time1).count() << "ms"
                      << std::endl;
            std::cout << "============================" << std::endl;
        }
//        std::vector<char> result = game_solver.SolveGame(*game_level_ptr_, destination_record_);
    }

    GameState GetGameState() const { return game_state_; }

private:
    GameResource game_resource_;
    std::unordered_map<GameResource::ObjectName, GLuint> obj_name_to_texture_id_;
    int step_count_ = 0;

    bool MovePlayer(int move_x, int move_y);

    void RenderPlayingState(float game_area_height = 300, float game_area_width = 300);

    void RenderStartState();

    void RenderFunctionButtons();

    int SetButtonStyle();

    void RenderLevelEditor(float game_area_height = 300, float game_area_width = 300);

    template<typename Iterable1, typename Iterable2>
    void DrawGamePlaying(const Iterable1 &box_positions_list, const Iterable2 &destination_list,
                         const GameResource::Position &player_position, float game_area_height,
                         float game_area_width, float top_left_pos_x = -1, float top_left_pos_y = -1,
                         bool show_player = true);

    void PrepareLevelEditor();

    void LoadGameLevelData(GameResource::GameLevelData &level_data);

    using Position = GameResource::Position;
    std::vector<std::vector<bool>> destination_record_;
    std::set<Position> box_pos_record_;
    Position player_position_;

    int count_destination_left_;
    int map_num_rows_, map_num_cols_;

    GameState game_state_;

    struct GameRecordNode {
        Position player_position;
        std::vector<std::vector<bool>> destination_record;
        std::set<Position> box_position_record;
        int count_destination_left;
        GameState game_state;
        int count_steps_;

        GameRecordNode(const Position &player_position, std::set<Position> box_position_record,
                       const std::vector<std::vector<bool>> &destination_record, int count_destination_left,
                       GameState game_state, int count_steps)
                : player_position(player_position), box_position_record(box_position_record),
                  destination_record(destination_record), count_destination_left(count_destination_left),
                  game_state(game_state), count_steps_(count_steps) {}

        void ReadFromRecord(PushBox &push_box) {
            push_box.player_position_ = player_position;
            push_box.destination_record_ = destination_record;
            push_box.box_pos_record_ = box_position_record;
            push_box.count_destination_left_ = count_destination_left;
            push_box.game_state_ = game_state;
            push_box.step_count_ = count_steps_;
        }
    };

    struct GameRecord {
        GameRecord(int max_record_size = 50) : max_record_size(max_record_size) {}

        void Record(const PushBox &push_box) {
            if (player_operation_record.size() >= max_record_size) {
                player_operation_record.pop_front();
            }
            player_operation_record.emplace_back(push_box.player_position_, push_box.box_pos_record_,
                                                 push_box.destination_record_,
                                                 push_box.count_destination_left_, push_box.game_state_,
                                                 push_box.step_count_);
        }

        /**
         * 
         * @return true if revoke successfully. false when record is empty. 
         */
        bool RecoverRecord(PushBox &push_box) {
            if (player_operation_record.empty()) {
                return false;
            }
            player_operation_record.back().ReadFromRecord(push_box);
            player_operation_record.pop_back();
            return true;
        }

        void ClearRecord() {
            player_operation_record.clear();
        }

        int max_record_size;
        std::deque<GameRecordNode> player_operation_record;
    };

    GameRecord game_record_{50};
    GameRecord game_record_for_show_solution_;

    void RenderWin(float game_area_height, float game_area_width);

    void ShowSolutionNextStep();
    void ShowSolutionPreviousStep();

    bool show_win_image_ = true;

    bool resume_game_flag_ = false;

    LevelEditor level_editor_;

    BFS_Solver game_solver;
    int selected_level_id = 0;
    GameResource::GameLevelData *game_level_ptr_;
    bool solver_working = false;
    std::vector<char> bfs_result;
    bool show_solution_steps_by_steps;
    int show_solution_steps;
};


#endif //DEMO_IMGUI_PUSHBOX_H
