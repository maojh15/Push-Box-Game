//
// Created by 冒家豪 on 2023/8/27.
//

#include "PushBox.h"
#include "imgui.h"

void PushBox::Render(int game_area_height, int game_area_width) {
    int block_edge_len = std::min(game_area_height / game_resource_.map_num_rows,
                                  game_area_width / game_resource_.map_num_cols);
    ImVec2 block_size(block_edge_len + 1, block_edge_len + 1);
    auto &io = ImGui::GetIO();
    ImVec2 left_top_pos(static_cast<int>((io.DisplaySize.x - game_resource_.map_num_cols * block_edge_len) / 2),
                        static_cast<int>((io.DisplaySize.y - game_resource_.map_num_rows * block_edge_len) / 2));

    box_positions_.clear();
    for (int i = 0; i < level_data_ptr_.size(); ++i) {
        for (int j = 0; j < level_data_ptr_[i].size(); ++j) {
            if (level_data_ptr_[i][j] == GameResource::OUTSIDE) {
                continue;
            }
            ImGui::SetCursorPos(ImVec2(left_top_pos.x + j * block_edge_len, left_top_pos.y + i * block_edge_len));
            ImGui::Image((void *) (intptr_t) (game_resource_.GetFloorTexture()), block_size);
            ImGui::SetCursorPos(ImVec2(left_top_pos.x + j * block_edge_len, left_top_pos.y + i * block_edge_len));
            switch (level_data_ptr_[i][j]) {
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
        ImGui::Image((void *) (intptr_t) (
                destination_record_[pos.y][pos.x] ?
                game_resource_.GetBoxArrivedTexture() :
                game_resource_.GetBoxTexture()), block_size);
    }


    ImGui::SetCursorPos(comp_position(player_position_));
    ImGui::Image((void *) (intptr_t) (game_resource_.GetPlayerTextuer()), block_size,
                 game_resource_.player_face_uv0, game_resource_.player_face_uv1);

    CheckGameState();

    if (game_state_ == WIN) {
        RenderWin();
    }

    ImGui::SetCursorPos(ImVec2(io.DisplaySize.x - 100, 0.0));
    ImGui::TextColored(ImVec4(0.0, 0.0, 0.0, 1.0), "step: %d", step_count_);
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

/**
 *
 * @param move_x
 * @param move_y
 * @return whether state of game date recorded is changed.
 */
bool PushBox::MovePlayer(int move_x, int move_y) {
    Position moved_pos{player_position_.x + move_x, player_position_.y + move_y};
    if (level_data_ptr_[moved_pos.y][moved_pos.x] == GameResource::WALL) {
        return false;
    }
    bool is_recorded = false;
    if (level_data_ptr_[moved_pos.y][moved_pos.x] == GameResource::BOX) {
        // move box
        if (level_data_ptr_[moved_pos.y + move_y][moved_pos.x + move_x] != GameResource::FLOOR) {
            return false;
        }
        game_record_.Record(*this);
        is_recorded = true;
        if (destination_record_[moved_pos.y][moved_pos.x]) {
            ++count_destination_left_;
        }
        if (destination_record_[moved_pos.y + move_y][moved_pos.x + move_x]) {
            --count_destination_left_;
        }
        level_data_ptr_[moved_pos.y + move_y][moved_pos.x + move_x] = GameResource::BOX;
    }
    if (!is_recorded) {
        game_record_.Record(*this);
    }
    // move player
    level_data_ptr_[player_position_.y][player_position_.x] = GameResource::FLOOR;
    player_position_ = moved_pos;
    level_data_ptr_[player_position_.y][player_position_.x] = GameResource::PLAYER;
    ++step_count_;
    return true;
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

void PushBox::ResetGame() {
    player_position_ = game_resource_.initial_player_position;
    destination_positions_ = game_resource_.initial_destination_positions;
    level_data_ptr_ = game_resource_.level_data;
    destination_record_ = game_resource_.destination_record;
    count_destination_left_ = destination_positions_.size();
    game_state_ = PLAYING;
    game_record_.ClearRecord();
    step_count_ = 0;
    show_win_image_ = true;
}

void PushBox::RenderWin() {
    if (show_win_image_) {
        auto win_texture = game_resource_.GetWinTextureObj();
        auto io = ImGui::GetIO();
        int txt_width = 0.5 * io.DisplaySize.x;
        int txt_height = 0.5 * io.DisplaySize.y;
        ImGui::SetCursorPos(
                ImVec2((io.DisplaySize.x - txt_width) / 2, (io.DisplaySize.y - txt_height) / 2));
        ImGui::Image((void *) (intptr_t) (win_texture->textureID), ImVec2(txt_width, txt_height));
    }

    // Add toggle win button.
    auto io = ImGui::GetIO();
    int button_width = 200;
    int button_height = 40;
    ImGui::SetCursorPos(ImVec2((io.DisplaySize.x - button_width) / 2,
                               io.DisplaySize.y - 10 - button_height));
    std::string hide_win = "Hide 'win'";
    std::string show_win = "Show 'win'";
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4) ImColor::HSV(6 / 7.0f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4) ImColor::HSV(7 / 7.0f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4) ImColor::HSV(7 / 7.0f, 0.8f, 0.8f));
    if (ImGui::Button((show_win_image_ ? hide_win : show_win).c_str(), ImVec2(button_width, button_height))) {
        show_win_image_ = !show_win_image_;
    }
    ImGui::PopStyleColor(3);
}