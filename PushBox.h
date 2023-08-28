//
// Created by 冒家豪 on 2023/8/27.
//

#ifndef DEMO_IMGUI_PUSHBOX_H
#define DEMO_IMGUI_PUSHBOX_H

#include "GameResource.h"
#include "SDL.h"

class PushBox {
public:
    PushBox() {
        ResetGame();
    }

    void Render(int game_area_height = 300, int game_area_width = 300);

    GameResource GetGameResource() { return game_resource_; }

    void ProcessInput(SDL_Event &event);

    void CheckGameState();

    void ResetGame();

    void RevokeOneStep() {
        game_record_.RecoverRecord(*this);
    }

private:
    GameResource game_resource_;
    std::unordered_map<GameResource::ObjectName, GLuint> obj_name_to_texture_id_;

    bool MovePlayer(int move_x, int move_y);

    using Position = GameResource::Position;
    Position player_position_;
    std::vector<Position> destination_positions_, box_positions_;
    std::vector<std::vector<GameResource::ObjectName>> level_data_ptr_;
    std::vector<std::vector<bool>> destination_record_;

    int count_destination_left_;

    enum GameState {
        PLAYING, PREWIN, WIN
    };

    GameState game_state_;

    struct GameRecordNode {
        Position player_position;
        std::vector<Position> destination_positions;
        std::vector<std::vector<GameResource::ObjectName>> level_data;
        std::vector<std::vector<bool>> destination_record;
        int count_destination_left;
        GameState game_state;

        GameRecordNode(const Position &player_position, const std::vector<Position> &destination_positions,
                       const std::vector<std::vector<GameResource::ObjectName>> &level_data,
                       const std::vector<std::vector<bool>> &destination_record, int count_destination_left,
                       GameState game_state)
                : player_position(player_position), destination_positions(destination_positions),
                  level_data(level_data), destination_record(destination_record),
                  count_destination_left(count_destination_left), game_state(game_state) {}

        void ReadFromRecord(PushBox &push_box) {
            push_box.player_position_ = player_position;
            push_box.destination_positions_ = destination_positions;
            push_box.level_data_ptr_ = level_data;
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
            player_operation_record.emplace_back(push_box.player_position_, push_box.destination_positions_,
                                                 push_box.level_data_ptr_, push_box.destination_record_,
                                                 push_box.count_destination_left_, push_box.game_state_);
        }

        void RecoverRecord(PushBox &push_box) {
            std::cout << player_operation_record.size() << std::endl;
            if (player_operation_record.empty()) {
                return;
            }
            player_operation_record.back().ReadFromRecord(push_box);
            player_operation_record.pop_back();
        }

        void ClearRecord() {
            player_operation_record.clear();
        }

        int max_record_size;
        std::deque<GameRecordNode> player_operation_record;
    };

    GameRecord game_record_{50};

};


#endif //DEMO_IMGUI_PUSHBOX_H
