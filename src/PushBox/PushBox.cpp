//
// Created by 冒家豪 on 2023/8/27.
//

#include "PushBox.h"
#include "imgui.h"

void PushBox::Render(int game_area_height, int game_area_width) {
    switch (game_state_) {
        case START:
            RenderStartState();
            break;
        case PLAYING:
            RenderPlayingState(game_area_height, game_area_width);
            break;
        case PREWIN:
        case WIN:
            RenderWin(game_area_height, game_area_width);
            break;
        case LEVEL_EDITOR:
            RenderLevelEditor(game_area_height, game_area_width);
            break;
        default:
            break;
    }
}

void PushBox::RenderPlayingState(int game_area_height, int game_area_width) {
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

    if (game_state_ != WIN) {
        CheckGameState();
    }

    ImGui::SetCursorPos(ImVec2(io.DisplaySize.x - 100, 0.0));
    ImGui::TextColored(ImVec4(0.0, 0.0, 0.0, 1.0), "step: %d", step_count_);

    RenderFunctionButtons();
}

void PushBox::RenderFunctionButtons() {
    // Add Button
    int num_style = SetButtonStyle();
    ImVec2 button_sz(200, 50);
    if (ImGui::Button("Return Title", button_sz)) {
        game_state_ = START;
        resume_game_flag_ = true;
    }

    button_sz.y = 40;
    if (ImGui::Button("Restart", button_sz)) {
        InitializeGame();
    }
    button_sz.y = 50;
    if (ImGui::Button("Revoke One Step.", button_sz)) {
        RevokeOneStep();
    }
    // if (ImGui::Button("BFS Solution", ImVec2(200, 30))) {
    //     push_box_game_machine.BFSSolveGame();
    // }
    ImGui::PopStyleColor(num_style);
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
        return;
    }
    game_state_ = PREWIN;
}

void PushBox::InitializeGame() {
    player_position_ = game_resource_.initial_player_position;
    destination_positions_ = game_resource_.initial_destination_positions;
    level_data_ptr_ = game_resource_.level_data;
    destination_record_ = game_resource_.destination_record;
    count_destination_left_ = destination_positions_.size();
    game_state_ = PLAYING;
    game_record_.ClearRecord();
    step_count_ = 0;
    show_win_image_ = true;
    resume_game_flag_ = false;
}

void PushBox::RenderWin(int game_area_height, int game_area_width) {
    RenderPlayingState(game_area_height, game_area_width);

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
    int num_style = SetButtonStyle();
    if (ImGui::Button((show_win_image_ ? hide_win : show_win).c_str(), ImVec2(button_width, button_height))) {
        show_win_image_ = !show_win_image_;
    }
    ImGui::PopStyleColor(num_style);
}

//int SetButtonStyleForEnd() {
//    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4) ImColor::HSV(7 / 8.0f, 0.6f, 0.8f, 1));
//    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4) ImColor::HSV(7 / 8.0f, 0.7f, 0.85f, 1));
//    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4) ImColor::HSV(7 / 8.0f, 0.8f, 0.9f, 1));
//    return 3;
//}

void PushBox::RenderStartState() {
    int num_style = SetButtonStyle();

    ImVec2 button_sz(200, 40);
    auto &io = ImGui::GetIO();

    ImGui::SetCursorPosX(io.DisplaySize.x - button_sz.x - 10);
    ImGui::SetCursorPosY(10);
    if (ImGui::Button("Level Editor", button_sz)) {
        game_state_ = LEVEL_EDITOR;
    }

    ImGui::SetCursorPosX((io.DisplaySize.x - button_sz.x) / 2);

    if (resume_game_flag_) {
        ImGui::SetCursorPosY(0.25 * io.DisplaySize.y - button_sz.y);
        if (ImGui::Button("Resume Game", button_sz)) {
            game_state_ = PLAYING;
        }
    } else {
        ImGui::SetCursorPosY(0.25 * io.DisplaySize.y);
    }

    button_sz.y = 60;
    button_sz.x = 180;
    ImGui::SetCursorPosX((io.DisplaySize.x - button_sz.x) / 2);
    if (ImGui::Button("Start Game", button_sz)) {
        InitializeGame();
    }

//    ImGui::PopStyleColor(num_style);
//
//    num_style = SetButtonStyleForEnd();
    ImGui::SetCursorPosX((io.DisplaySize.x - button_sz.x) / 2);
    if (ImGui::Button("End Game", button_sz)) {
        game_state_ = END;
    }

    ImGui::PopStyleColor(num_style);

    ImGui::SetCursorPosX(io.DisplaySize.x - 130);
    ImGui::SetCursorPosY(io.DisplaySize.y - 40);
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(1, 1, 1, 0.6));
    ImGui::TextColored(ImVec4(0, 0, 0, 1), GAME_VERSION);
    ImGui::PopStyleColor();
}

int PushBox::SetButtonStyle() {
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4) ImColor::HSV(7 / 8.0f, 0.6f, 0.8f, 1));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4) ImColor::HSV(7.5 / 8.0f, 0.7f, 0.95f, 1));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4) ImColor::HSV(7 / 8.0f, 0.8f, 0.9f, 1));
    return 3;
}

void PushBox::RenderLevelEditor(int game_area_height, int game_area_width) {

}

