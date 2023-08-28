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
        player_position_ = game_resource_.initial_player_position;
        destination_positions_ = game_resource_.initial_destination_positions;
        level_data_ptr_ = &game_resource_.level_data;
        destination_record_ = &game_resource_.destination_record;
        count_destination_left_ = destination_positions_.size();
        game_state_ = PLAYING;
    }

    void Render(int game_area_height = 300, int game_area_width = 300);

    GameResource GetGameResource() { return game_resource_; }

    void ProcessInput(SDL_Event &event);

    void CheckGameState();

private:
    GameResource game_resource_;
    std::unordered_map<GameResource::ObjectName, GLuint> obj_name_to_texture_id_;

    void MovePlayer(int move_x, int move_y);

    using Position = GameResource::Position;
    Position player_position_;
    std::vector<Position> destination_positions_, box_positions_;
    std::vector<std::vector<GameResource::ObjectName>> *level_data_ptr_;
    std::vector<std::vector<bool>> *destination_record_;

    int count_destination_left_;

    enum GameState {
        PLAYING, PREWIN, WIN
    };

    GameState game_state_;
};


#endif //DEMO_IMGUI_PUSHBOX_H
