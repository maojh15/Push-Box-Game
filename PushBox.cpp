//
// Created by 冒家豪 on 2023/8/27.
//

#include "PushBox.h"
#include "imgui.h"

void PushBox::Render(int game_area_height, int game_area_width) {
    int block_edge_len = std::min(game_area_height / game_resource_.map_num_rows,
                                  game_area_width / game_resource_.map_num_cols);
    ImVec2 block_size(block_edge_len + 2, block_edge_len + 1);
    auto &io = ImGui::GetIO();
    ImVec2 left_top_pos((io.DisplaySize.x - game_resource_.map_num_cols * block_edge_len) / 2,
                        (io.DisplaySize.y - game_resource_.map_num_rows * block_edge_len) / 2);

    box_positions_.clear();
    for (int i = 0; i < game_resource_.level_data.size(); ++i) {
        for (int j = 0; j < game_resource_.level_data[i].size(); ++j) {
            ImGui::SetCursorPos(ImVec2(left_top_pos.x + j * block_edge_len, left_top_pos.y + i * block_edge_len));
            ImGui::Image((void *) (intptr_t) (game_resource_.GetFloorTexture()), block_size);
            ImGui::SetCursorPos(ImVec2(left_top_pos.x + j * block_edge_len, left_top_pos.y + i * block_edge_len));
            switch (game_resource_.level_data[i][j]) {
                case GameResource::WALL:
                    ImGui::Image((void *) (intptr_t) (game_resource_.GetWallTexture()), block_size);
                    break;
                case GameResource::BOX:
                    box_positions_.emplace_back(j, i);
                    break;
                case GameResource::DESTINATION:
                    break;
                case GameResource::PLAYER:
                    break;
                default:
                    break;
            }
        }
    }
    auto comp_position = [&left_top_pos, block_edge_len](const Position &pos) {
        return ImVec2(left_top_pos.x + pos.x * block_edge_len,
                      left_top_pos.y + pos.y * block_edge_len);
    };

    for (auto &pos: destination_positions_) {
        ImGui::SetCursorPos(comp_position(pos));
        ImGui::Image((void *) (intptr_t) (game_resource_.GetDestinationTexture()), block_size);
    }
    for (auto &pos: box_positions_) {
        ImGui::SetCursorPos(comp_position(pos));
        ImGui::Image((void *) (intptr_t) (game_resource_.GetBoxTexture()), block_size);
    }


    ImGui::SetCursorPos(comp_position(player_position_));
    ImGui::Image((void *) (intptr_t) (game_resource_.GetPlayerTextuer()), block_size,
                 game_resource_.player_face_uv0, game_resource_.player_face_uv1);

    CheckGameState();

    if (game_state_ == WIN) {
        auto win_texture = game_resource_.GetWinTextureObj();
        auto io = ImGui::GetIO();
        int txt_width = 0.5 * io.DisplaySize.x;
        int txt_height = 0.5 * io.DisplaySize.y;
        ImGui::SetCursorPos(
                ImVec2((io.DisplaySize.x - txt_width) / 2, (io.DisplaySize.y - txt_height) / 2));
        ImGui::Image((void *) (intptr_t) (win_texture->textureID), ImVec2(txt_width, txt_height));
    }
}

void PushBox::ProcessInput(SDL_Event &event) {
    if (game_state_ == PLAYING && event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_w:
            case SDLK_UP:
                MovePlayer(0, -1);
                break;
            case SDLK_a:
            case SDLK_LEFT:
                MovePlayer(-1, 0);
                break;
            case SDLK_s:
            case SDLK_DOWN:
                MovePlayer(0, 1);
                break;
            case SDLK_d:
            case SDLK_RIGHT:
                MovePlayer(1, 0);
                break;
        }
    }
}

void PushBox::MovePlayer(int move_x, int move_y) {
    Position moved_pos{player_position_.x + move_x, player_position_.y + move_y};
    if ((*level_data_ptr_)[moved_pos.y][moved_pos.x] == GameResource::WALL) {
        return;
    }
    if ((*level_data_ptr_)[moved_pos.y][moved_pos.x] == GameResource::BOX) {
        // move box
        if ((*level_data_ptr_)[moved_pos.y + move_y][moved_pos.x + move_x] != GameResource::FLOOR) {
            return;
        }
        if ((*destination_record_)[moved_pos.y][moved_pos.x]) {
            ++count_destination_left_;
        }
        if ((*destination_record_)[moved_pos.y + move_y][moved_pos.x + move_x]) {
            --count_destination_left_;
        }
        (*level_data_ptr_)[moved_pos.y + move_y][moved_pos.x + move_x] = GameResource::BOX;
    }
    // move player
    (*level_data_ptr_)[player_position_.y][player_position_.x] = GameResource::FLOOR;
    player_position_ = moved_pos;
    (*level_data_ptr_)[player_position_.y][player_position_.x] = GameResource::PLAYER;
    std::cout << "count_destination_left_ = " << count_destination_left_ << std::endl;
}

void PushBox::CheckGameState() {
    if (count_destination_left_ != 0) {
        return;
    }
    if (game_state_ == PREWIN) {
        SDL_Delay(500);
        game_state_ = WIN;
    }
    if (game_state_ != WIN) {
        game_state_ = PREWIN;
    }
}