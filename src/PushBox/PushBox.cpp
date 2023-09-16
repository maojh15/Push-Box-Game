//
// Created by 冒家豪 on 2023/8/27.
//

#include "PushBox.h"
#include "imgui.h"

void PushBox::Render(float game_area_height, float game_area_width) {
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

void PushBox::RenderPlayingState(float game_area_height, float game_area_width) {
    DrawGamePlaying(box_pos_record_, game_level_ptr_->destination_positions, player_position_, game_area_height,
                    game_area_width);

    if (game_state_ != WIN) {
        CheckGameState();
    }

    auto &io = ImGui::GetIO();
    ImGui::SetCursorPos(ImVec2(io.DisplaySize.x - 100, 0.0));
    ImGui::TextColored(ImVec4(0.0, 0.0, 0.0, 1.0), "step: %d", step_count_);

    RenderFunctionButtons();
}

template<typename Iterable1, typename Iterable2>
void PushBox::DrawGamePlaying(const Iterable1 &box_positions_list, const Iterable2 &destination_list,
                              const Position &player_position, float game_area_height, float game_area_width,
                              float top_left_pos_x, float top_left_pos_y,
                              bool show_player) {
    float block_edge_len = std::min(game_area_height / map_num_rows_,
                                    game_area_width / map_num_cols_);
    const float MAX_BLOCK_LEN = 50;
    block_edge_len = std::min(block_edge_len, MAX_BLOCK_LEN);
    ImVec2 block_size(block_edge_len + 1, block_edge_len + 1);
    auto &io = ImGui::GetIO();
    ImVec2 left_top_pos;
    if (top_left_pos_x < 0 || top_left_pos_y < 0) {
        left_top_pos = ImVec2(((io.DisplaySize.x - map_num_cols_ * block_edge_len) / 2),
                              ((io.DisplaySize.y - map_num_rows_ * block_edge_len) / 2));
    } else {
        left_top_pos = ImVec2(top_left_pos_x, top_left_pos_y);
    }

    if (game_state_ == LEVEL_EDITOR) {
        level_editor_.SetDrawParameters(left_top_pos.x, left_top_pos.y, block_edge_len);
    }

    for (int i = 0; i < game_level_ptr_->game_wall_map.size(); ++i) {
        for (int j = 0; j < game_level_ptr_->game_wall_map[i].size(); ++j) {
            if (game_level_ptr_->game_wall_map[i][j] == GameResource::OUTSIDE) {
                continue;
            }
            ImGui::SetCursorPos(ImVec2(left_top_pos.x + j * block_edge_len, left_top_pos.y + i * block_edge_len));
            ImGui::Image((void *) (intptr_t) (game_resource_.GetFloorTexture()), block_size);
            ImGui::SetCursorPos(ImVec2(left_top_pos.x + j * block_edge_len, left_top_pos.y + i * block_edge_len));
            switch (game_level_ptr_->game_wall_map[i][j]) {
                case GameResource::WALL:
                    ImGui::Image((void *) (intptr_t) (game_resource_.GetWallTexture()), block_size);
                    break;
                case GameResource::BOX:
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

    for (auto &pos: destination_list) {
        ImGui::SetCursorPos(comp_position(pos));
        ImGui::Image((void *) (intptr_t) (game_resource_.GetDestinationTexture()), block_size);
    }
    for (auto &pos: box_positions_list) {
        ImGui::SetCursorPos(comp_position(pos));
        ImGui::Image((void *) (intptr_t) (
                destination_record_[pos.y][pos.x] ?
                game_resource_.GetBoxArrivedTexture() :
                game_resource_.GetBoxTexture()), block_size);
    }

    if (show_player) {
        ImGui::SetCursorPos(comp_position(player_position));
        ImGui::Image((void *) (intptr_t) (game_resource_.GetPlayerTextuer()), block_size,
                     game_resource_.player_face_uv0, game_resource_.player_face_uv1);
    }
}

GameResource::Position GetMovingFromBFS_Result(const std::vector<char> &bfs_result, int step_index) {
    switch (bfs_result[step_index]) {
        case 'u':
            return GameResource::Position(0, -1);
            break;
        case 'r':
            return GameResource::Position(1, 0);
            break;
        case 'd':
            return GameResource::Position(0, 1);
            break;
        case 'l':
            return GameResource::Position(-1, 0);
            break;
        default:
            throw std::invalid_argument("Invalid char " + std::to_string(bfs_result[step_index]) + " in bfs_result");
    }
}

void PushBox::RenderFunctionButtons() {
    // Add Button
    int num_style = SetButtonStyle();
    if (solver_working || show_solution_steps_by_steps) {
        ImGui::BeginDisabled(true);
    }
    ImVec2 button_sz(170, 50);
    if (ImGui::Button("Return Title", button_sz)) {
        if (game_state_ == PLAYING) {
            resume_game_flag_ = true;
            game_record_.Record(*this);
        }
        game_state_ = START;
    }

    auto &io = ImGui::GetIO();
    button_sz.y = 50;
    if (ImGui::Button("Revoke One Step.", button_sz)) {
        RevokeOneStep();
    }
    if (solver_working || show_solution_steps_by_steps) {
        ImGui::EndDisabled();
    }
    if (!game_solver.solver_done_flag) {
        if (!solver_working && ImGui::Button("BFS Solution", button_sz)) {
            BFSSolveGame();
        }
    } else {
        ImVec2 show_sol_button_sz(150, button_sz.y);
        ImGui::SetCursorPosX(io.DisplaySize.x - show_sol_button_sz.x - 10);
        ImGui::SetCursorPosY(50);
        if (!show_solution_steps_by_steps && ImGui::Button("Show Solution", show_sol_button_sz)) {
            show_solution_steps_by_steps = true;
            show_solution_steps = 0;
            game_record_.Record(*this);
            std::swap(game_record_, game_record_for_show_solution_);
            LoadGameLevelData(game_resource_.GetLevelData(selected_level_id));
        } else if (show_solution_steps_by_steps) {
            ImGui::SetCursorPosX(io.DisplaySize.x - show_sol_button_sz.x - 10);
            if (ImGui::Button("Return playing", show_sol_button_sz)) {
                show_solution_steps_by_steps = false;
                std::swap(game_record_, game_record_for_show_solution_);
                game_record_.RecoverRecord(*this);
            }
        }
        const std::string hint_tot_steps_sol("total steps of solution");
        auto text_sz = ImGui::CalcTextSize(hint_tot_steps_sol.c_str());
        const std::string hint_steps = std::to_string(bfs_result.size());
        auto text_steps_sz = ImGui::CalcTextSize(hint_steps.c_str());
        ImGui::SetCursorPosX(io.DisplaySize.x - text_sz.x - 10);
        ImGui::TextColored(ImVec4(0, 0, 0, 1), "%s", hint_tot_steps_sol.c_str());
        ImGui::SetCursorPosX(io.DisplaySize.x - (text_sz.x + text_steps_sz.x) / 2 - 10);
        ImGui::TextColored(ImVec4(0, 0, 0, 1), "%s", hint_steps.c_str());

        if (show_solution_steps_by_steps) {
            ImGui::BeginDisabled(show_solution_steps <= 0);
            ImGui::SetCursorPosX(io.DisplaySize.x - show_sol_button_sz.x - 10);
            if (ImGui::Button("Previous Step", show_sol_button_sz)) {
                game_record_.RecoverRecord(*this);
                --show_solution_steps;
            }
            ImGui::EndDisabled();
            ImGui::BeginDisabled(show_solution_steps + 1 > bfs_result.size());
            ImGui::SetCursorPosX(io.DisplaySize.x - show_sol_button_sz.x - 10);
            if (ImGui::Button("Next step", show_sol_button_sz)) {
                auto moving = GetMovingFromBFS_Result(bfs_result, show_solution_steps);
                MovePlayer(moving.x, moving.y);
                ++show_solution_steps;
            }
            ImGui::EndDisabled();
        }
    }

    button_sz.y = 40;
    if (solver_working || show_solution_steps_by_steps) {
        ImGui::BeginDisabled(true);
    }
    ImGui::SetCursorPosY(io.DisplaySize.y * 0.5 - button_sz.y);
    if (ImGui::Button("Previous Level", button_sz)) {
        int sz = game_resource_.GetListLevels().size();
        selected_level_id = (selected_level_id + sz - 1) % sz;
        InitializeGame();
    }
    if (ImGui::Button("Next Level", button_sz)) {
        selected_level_id = (selected_level_id + 1) % game_resource_.GetListLevels().size();
        InitializeGame();
    }

    ImGui::SetCursorPosY(io.DisplaySize.y - button_sz.y - 10);
    if (ImGui::Button("Restart", button_sz)) {
        InitializeGame();
    }
    if (solver_working || show_solution_steps_by_steps) {
        ImGui::EndDisabled();
    }

    if (solver_working) {
        BFSSolveGame();
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.216, 0.27, 0.178, 0.873));
        ImGui::SetNextWindowSize(ImVec2(120, 100), ImGuiCond_Always);
        ImGui::Begin("Solving ...", nullptr,
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoTitleBar);
        ImGui::Text("Solving ... ");
        ImVec2 abrupt_button_sz(100, 40);
        if (ImGui::Button("Abrupt", abrupt_button_sz)) {
            game_solver.abrupt_flag = true;
            solver_working = false;
        }
        ImGui::End();
        ImGui::PopStyleColor();
    }

    ImGui::SetCursorPos(ImVec2(io.DisplaySize.x - 80, io.DisplaySize.y - 60));
    ImGui::TextColored(ImVec4(0, 0, 0, 1), "Lv %d.", selected_level_id);
    ImGui::PopStyleColor(num_style);
}

/**
 *
 * @param event
 * @return false, iff the event should be eaten
 */
bool PushBox::ProcessInput(SDL_Event &event) {
    if (solver_working || show_solution_steps_by_steps) {
        if (game_state_ == PLAYING && event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_w:
                case SDLK_UP:
                case SDLK_d:
                case SDLK_RIGHT:
                case SDLK_s:
                case SDLK_DOWN:
                case SDLK_a:
                case SDLK_LEFT:
                    return false;
                    break;
                default:
                    break;
            }
        }
        return true;
    }
    if (game_state_ == LEVEL_EDITOR) {
        if (event.type == SDL_MOUSEMOTION) {
            level_editor_.SetMousePosition(event.motion.x, event.motion.y);
        } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == 1) {
            level_editor_.mouse_button1_down = true;
        } else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == 1) {
            level_editor_.mouse_button1_down = false;
        }
    } else if (game_state_ == PLAYING && event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_w:
            case SDLK_UP:
                MovePlayer(0, -1);
                return false;
                break;
            case SDLK_a:
            case SDLK_LEFT:
                MovePlayer(-1, 0);
                return false;
                break;
            case SDLK_s:
            case SDLK_DOWN:
                MovePlayer(0, 1);
                return false;
                break;
            case SDLK_d:
            case SDLK_RIGHT:
                MovePlayer(1, 0);
                return false;
                break;
        }
    }
    return true;
}

/**
 *
 * @param move_x
 * @param move_y
 * @return whether state of game date recorded is changed.
 */
bool PushBox::MovePlayer(int move_x, int move_y) {
    Position moved_pos{player_position_.x + move_x, player_position_.y + move_y};
    if (game_level_ptr_->game_wall_map[moved_pos.y][moved_pos.x] == GameResource::WALL) {
        return false;
    }
    bool is_recorded = false;
    auto box_itr = box_pos_record_.find(moved_pos);
    if (box_itr != box_pos_record_.end()) {
        Position moved_box_pos(moved_pos.x + move_x, moved_pos.y + move_y);
        // move box
        if (game_level_ptr_->game_wall_map[moved_box_pos.y][moved_box_pos.x] == GameResource::WALL ||
            box_pos_record_.find(moved_box_pos) != box_pos_record_.end()) {
            return false;
        }
        // record before change game state.
        game_record_.Record(*this);
        is_recorded = true;

        box_pos_record_.erase(box_itr);
        box_pos_record_.insert(moved_box_pos);
        if (destination_record_[moved_pos.y][moved_pos.x]) {
            ++count_destination_left_;
        }
        if (destination_record_[moved_box_pos.y][moved_box_pos.x]) {
            --count_destination_left_;
        }
    }
    if (!is_recorded) {
        game_record_.Record(*this);
    }
    // move player
    player_position_ = moved_pos;
    ++step_count_;
    return true;
}

void PushBox::CheckGameState() {
    if (count_destination_left_ != 0 || show_solution_steps_by_steps) {
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
    LoadGameLevelData(game_resource_.GetLevelData(selected_level_id));

    game_state_ = PLAYING;
    game_record_.ClearRecord();
    step_count_ = 0;
    show_win_image_ = true;
    resume_game_flag_ = false;
    game_solver.solver_done_flag = false;
    bfs_result.clear();
    show_solution_steps_by_steps = false;
    show_solution_steps = 0;
}

void PushBox::LoadGameLevelData(GameResource::GameLevelData &level_data) {
    game_level_ptr_ = &level_data;

    map_num_rows_ = game_level_ptr_->game_wall_map.size();
    map_num_cols_ = 0;
    for (auto &elem: game_level_ptr_->game_wall_map) {
        map_num_cols_ = std::max<int>(map_num_cols_, elem.size());
    }

    // initial player's position
    player_position_ = game_level_ptr_->player_position;
    // initial boxs' positions record.
    box_pos_record_.clear();
    for (auto &box_pos: game_level_ptr_->box_positions) {
        box_pos_record_.emplace(box_pos);
    }

    // initial destination record;
    destination_record_.resize(map_num_rows_);
    for (int i = 0; i < map_num_rows_; ++i) {
        destination_record_[i].resize(game_level_ptr_->game_wall_map[i].size());
        std::fill(destination_record_[i].begin(), destination_record_[i].end(), false);
    }
    for (auto &dest_pos: game_level_ptr_->destination_positions) {
        destination_record_[dest_pos.y][dest_pos.x] = true;
    }

    // initial destination number.
    count_destination_left_ = game_level_ptr_->destination_positions.size();
    for (const auto &box_pos: game_level_ptr_->box_positions) {
        if (destination_record_[box_pos.y][box_pos.x]) {
            --count_destination_left_;
        }
    }
}

void PushBox::RenderWin(float game_area_height, float game_area_width) {
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

void PushBox::PrepareLevelEditor() {
    level_editor_.ResetState();
    game_level_ptr_ = &level_editor_.game_level;
    level_editor_.destination_record_ptr_ = &destination_record_;
}

void PushBox::RenderStartState() {
    int num_style = SetButtonStyle();

    ImVec2 button_sz(200, 40);
    auto &io = ImGui::GetIO();

    ImGui::SetCursorPosX(io.DisplaySize.x - button_sz.x - 10);
    ImGui::SetCursorPosY(10);
    if (ImGui::Button("Level Editor", button_sz)) {
        // change to level_editor.
        PrepareLevelEditor();
        game_state_ = LEVEL_EDITOR;
    }

    ImGui::SetCursorPosX((io.DisplaySize.x - button_sz.x) / 2);

    if (resume_game_flag_) {
        ImGui::SetCursorPosY(0.25 * io.DisplaySize.y - button_sz.y);
        if (ImGui::Button("Resume Game", button_sz)) {
            game_state_ = PLAYING;
            resume_game_flag_ = false;
            LoadGameLevelData(game_resource_.GetLevelData(selected_level_id));
            game_record_.RecoverRecord(*this);
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

void PushBox::RenderLevelEditor(float game_area_height, float game_area_width) {
    int num_style = SetButtonStyle();
    ImGui::SetCursorPos(ImVec2(10, 20));
    ImVec2 button_sz(150, 40);
    if (ImGui::Button("Return Title", button_sz)) {
        game_state_ = START;
    }

    auto &io = ImGui::GetIO();

    if (!level_editor_.assigned_map_size) {
        level_editor_.RenderSetMapSize(map_num_rows_, map_num_cols_);
    }

    if (level_editor_.ShouldRenderGameMap()) {
        if (level_editor_.saved_) {
            DrawGamePlaying(level_editor_.game_level.box_positions,
                            level_editor_.game_level.destination_positions,
                            level_editor_.game_level.player_position, game_area_height,
                            game_area_width, -1, -1, level_editor_.GetShowPlayer());
        } else {
            DrawGamePlaying(level_editor_.box_positions_, level_editor_.destination_positions_,
                            level_editor_.game_level.player_position, game_area_height,
                            game_area_width, -1, -1, level_editor_.GetShowPlayer());


            ImGui::SetCursorPos(ImVec2(io.DisplaySize.x - button_sz.x - 10, 15));
            if (ImGui::Button("Save", button_sz)) {
                level_editor_.SaveLevelData();
                level_editor_.state_ = LevelEditor::SAVE_UI;
                LoadGameLevelData(level_editor_.game_level);
            }
        }
    }

    level_editor_.RenderEditor(game_area_height, game_area_width);
    if (level_editor_.state_ == LevelEditor::SAVED) {
        game_state_ = START;
    }

    ImGui::PopStyleColor(num_style);
}

