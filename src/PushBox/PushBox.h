//
// Created by 冒家豪 on 2023/8/27.
//

#ifndef DEMO_IMGUI_PUSHBOX_H
#define DEMO_IMGUI_PUSHBOX_H

#include "GameResource.h"
#include "SDL.h"
#include "BFS_Solver.h"

#define GAME_VERSION "version 1.0.1"

class PushBox {
public:
    enum GameState {
        START, PLAYING, PREWIN, WIN, END, LEVEL_EDITOR
    };

    PushBox() {
        game_state_ = START;
    }

    void Render(int game_area_height = 300, int game_area_width = 300);

    GameResource GetGameResource() { return game_resource_; }

    bool ProcessInput(SDL_Event &event);

    void CheckGameState();

    void InitializeGame();

    void RevokeOneStep() {
        if (game_record_.RecoverRecord(*this)) {
            --step_count_;
        }
    }

    void BFSSolveGame() {
        auto time1 = std::chrono::steady_clock::now();
        std::vector<char> result = game_solver.SolveGame(*game_level_ptr, destination_record_);
        auto time2 = std::chrono::steady_clock::now();
        int count_step = 0;
        std::cout << "======BFS Solution==========\n";
        std::string output_result;
        for (char ch: result) {
            output_result.push_back(ch);
            output_result.push_back(' ');
            ++count_step;
            if (count_step % 10 == 0) {
                output_result.push_back('\n');
            }
        }
        std::cout << output_result;
        std::cout << "\ntotal steps: " << result.size() << std::endl;
        std::cout << "cost time: " << std::chrono::duration<double, std::milli>(time2 - time1).count() << "ms"
                  << std::endl;
        std::cout << "============================" << std::endl;
    }

    GameState GetGameState() const { return game_state_; }

private:
    GameResource game_resource_;
    std::unordered_map<GameResource::ObjectName, GLuint> obj_name_to_texture_id_;
    int step_count_ = 0;

    bool MovePlayer(int move_x, int move_y);

    void RenderPlayingState(int game_area_height = 300, int game_area_width = 300);

    void RenderStartState();

    void RenderFunctionButtons();

    int SetButtonStyle();

    void RenderLevelEditor(int game_area_height = 300, int game_area_width = 300);

    using Position = GameResource::Position;
    std::vector<std::vector<bool>> destination_record_;
    std::vector<Position> box_positions_;
    Position player_position_;
    std::vector<std::vector<GameResource::ObjectName>> map_box_wall_floor_state_; // include information for box, floor and wall;

    int count_destination_left_;
    int map_num_rows_, map_num_cols_;

    GameState game_state_;

    struct GameRecordNode {
        Position player_position;
        std::vector<std::vector<GameResource::ObjectName>> map_box_wall_floor_state_;
        std::vector<std::vector<bool>> destination_record;
        int count_destination_left;
        GameState game_state;

        GameRecordNode(const Position &player_position,
                       const std::vector<std::vector<GameResource::ObjectName>> &map_box_wall_floor_state_,
                       const std::vector<std::vector<bool>> &destination_record, int count_destination_left,
                       GameState game_state)
                : player_position(player_position), map_box_wall_floor_state_(map_box_wall_floor_state_),
                  destination_record(destination_record), count_destination_left(count_destination_left),
                  game_state(game_state) {}

        void ReadFromRecord(PushBox &push_box) {
            push_box.player_position_ = player_position;
            push_box.map_box_wall_floor_state_ = map_box_wall_floor_state_;
            push_box.destination_record_ = destination_record;
            push_box.count_destination_left_ = count_destination_left;
            push_box.game_state_ = game_state;
        }
    };

    struct GameRecord {
        GameRecord(int max_record_size = 50) : max_record_size(max_record_size) {}

        void Record(const PushBox &push_box) {
            if (player_operation_record.size() >= max_record_size) {
                player_operation_record.pop_front();
            }
            player_operation_record.emplace_back(push_box.player_position_, push_box.map_box_wall_floor_state_,
                                                 push_box.destination_record_,
                                                 push_box.count_destination_left_, push_box.game_state_);
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

    void RenderWin(int game_area_height, int game_area_width);

    void RenderGamePlaying();

    bool show_win_image_ = true;

    bool resume_game_flag_ = false;

    BFS_Solver game_solver;
    int selected_level_id = 0;
    const GameResource::GameLevelData *game_level_ptr;
};


#endif //DEMO_IMGUI_PUSHBOX_H
