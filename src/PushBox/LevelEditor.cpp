//
// Created by 冒家豪 on 2023/9/13.
//

#include "imgui.h"
#include "LevelEditor.h"

#include <queue>
#include <set>

void LevelEditor::RenderSetMapSize(int &map_num_rows, int &map_num_cols) {
    auto &io = ImGui::GetIO();

    // set map size
    ImVec2 slide_sz(300, 40);
    ImVec2 button_sz(120, 40);
    ImGui::SetCursorPosX((io.DisplaySize.x - slide_sz.x) / 2);
    ImGui::SetCursorPosY(0.25 * io.DisplaySize.y);
    ImGui::PushItemWidth(slide_sz.x);
    ImGui::SliderInt("width of map", &map_n_cols, 5, 30, "%d");
    ImGui::SetCursorPosX((io.DisplaySize.x - slide_sz.x) / 2);
    ImGui::SliderInt("height of map", &map_n_rows, 5, 30, "%d");
    ImGui::PopItemWidth();

    ImGui::SetCursorPosX((io.DisplaySize.x - button_sz.x) / 2);
    if (ImGui::Button("OK", button_sz)) {
        assigned_map_size = true;
        // initialize
        game_level.Clear();
        game_level.game_wall_map.resize(map_n_rows);
        for (int i = 0; i < map_n_rows; ++i) {
            game_level.game_wall_map[i].resize(map_n_cols, GameResource::FLOOR);
        }
        map_num_rows = map_n_rows;
        map_num_cols = map_n_cols;

        // initialize destination_record;
        destination_record_ptr_->clear();
        destination_record_ptr_->resize(map_num_rows);
        for (auto &x: *destination_record_ptr_) {
            x.resize(map_num_cols, false);
        }
    }
}

void LevelEditor::ResetState() {
    assigned_map_size = false;
    is_player_pos_seted = false;
    game_level.Clear();
    selected_source_obj_ = GameResource::OUTSIDE;
    mouse_button1_down = false;
    box_positions_.clear();
    destination_positions_.clear();
    save_pressed_flag = false;
    state_ = DRAW_BRUSH;
    saved_ = false;
}

template<typename T>
void EraseElemIfExist(std::set<T> &any_sets, const T &element) {
    auto itr = any_sets.find(element);
    if (itr != any_sets.end()) {
        any_sets.erase(itr);
    }
}

void LevelEditor::RenderDrawBrush(float game_area_heigh, float game_area_with) {
    if (!assigned_map_size) {
        return;
    }
    ImVec2 block_size_vec2(block_len_, block_len_);

    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.53, 0.003, 0.623, 0.75));
    ImGui::SetCursorPos(ImVec2(10, 100));
    ImVec2 material_button_sz(50, 50);
    float curse_pos_x = 15;
    ImGui::SetCursorPosX(curse_pos_x);
    if (ImGui::ImageButton((void *) (intptr_t) game_resource_ptr_->GetWallTexture(), material_button_sz)) {
        selected_source_obj_ = GameResource::WALL;
    }
    ImGui::SetItemTooltip("Wall");
    ImGui::SetCursorPosX(curse_pos_x);
    if (ImGui::ImageButton((void *) (intptr_t) game_resource_ptr_->GetFloorTexture(), material_button_sz)) {
        selected_source_obj_ = GameResource::FLOOR;
    }
    ImGui::SetItemTooltip("Floor");
    ImGui::SetCursorPosX(curse_pos_x);
    if (ImGui::ImageButton((void *) (intptr_t) game_resource_ptr_->GetBoxTexture(), material_button_sz)) {
        selected_source_obj_ = GameResource::BOX;
    }
    ImGui::SetItemTooltip("Box");
    ImGui::SetCursorPosX(curse_pos_x);
    if (ImGui::ImageButton((void *) (intptr_t) game_resource_ptr_->GetDestinationTexture(), material_button_sz)) {
        selected_source_obj_ = GameResource::DESTINATION;
    }
    ImGui::SetItemTooltip("Destination");
    ImGui::SetCursorPosX(curse_pos_x);
    if (ImGui::ImageButton((void *) (intptr_t) game_resource_ptr_->GetPlayerTextuer(), material_button_sz,
                           game_resource_ptr_->player_face_uv0, game_resource_ptr_->player_face_uv1)) {
        selected_source_obj_ = GameResource::PLAYER;
    }
    ImGui::SetItemTooltip("Player");
    ImGui::PopStyleColor();

    auto *draw_list = ImGui::GetForegroundDrawList();

    float mouse_rel_x = mouse_x_ - top_left_pos_x_;
    float mouse_rel_y = mouse_y_ - top_left_pos_y_;
    int mouse_block_ind_x = static_cast<int>(mouse_rel_x / block_len_);
    int mouse_block_ind_y = static_cast<int>(mouse_rel_y / block_len_);
    if (mouse_rel_x >= 0 && mouse_block_ind_x < map_n_cols &&
        mouse_rel_y >= 0 && mouse_block_ind_y < map_n_rows) {
        ImVec2 draw_position(top_left_pos_x_ + mouse_block_ind_x * block_len_,
                             top_left_pos_y_ + mouse_block_ind_y * block_len_);
        ImGui::SetCursorPos(draw_position);
        switch (selected_source_obj_) {
            case GameResource::WALL:
                ImGui::Image((void *) (intptr_t) (game_resource_ptr_->GetWallTexture()), block_size_vec2);
                break;
            case GameResource::BOX:
                ImGui::Image((void *) (intptr_t) (game_resource_ptr_->GetBoxTexture()), block_size_vec2);
                break;
            case GameResource::FLOOR:
                ImGui::Image((void *) (intptr_t) (game_resource_ptr_->GetFloorTexture()), block_size_vec2);
                break;
            case GameResource::DESTINATION:
                ImGui::Image((void *) (intptr_t) (game_resource_ptr_->GetDestinationTexture()), block_size_vec2);
                break;
            case GameResource::PLAYER:
                ImGui::Image((void *) (intptr_t) (game_resource_ptr_->GetPlayerTextuer()), block_size_vec2,
                             game_resource_ptr_->player_face_uv0, game_resource_ptr_->player_face_uv1);
                break;
            default:
                break;
        }
        draw_list->AddRect(draw_position, ImVec2(draw_position.x + block_len_, draw_position.y + block_len_),
                           ImU32(ImColor(0, 0, 0, 255)), 2.0f);

        ProcessInput(mouse_block_ind_x, mouse_block_ind_y);
    }
}

void LevelEditor::RenderSaveUI() {
    const ImVec2 hint_window_size(400, 150);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.316, 0.47, 0.878, 0.973));
    const ImVec4 hint_text_color(1, 1, 1, 1);
    if (hint_str.size() != 0) {
        ImGui::SetNextWindowSize(hint_window_size, ImGuiCond_Always);
        ImGui::Begin("Hint", nullptr,
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar);
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + hint_window_size.x - 10);
        ImGui::TextColored(hint_text_color, "%s", hint_str.c_str());
        ImGui::PopTextWrapPos();
        const ImVec2 ok_button_sz(60, 40);
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ok_button_sz.x) / 2);
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ok_button_sz.y - 15);
        if (ImGui::Button("OK", ok_button_sz)) {
            state_ = DRAW_BRUSH;
        }
        ImGui::End();
    } else if (saved_) {
        ImGui::Begin("Hint", nullptr,
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + hint_window_size.x - 10);
        ImGui::TextColored(hint_text_color, "%s", "Level saved.");
        ImGui::PopTextWrapPos();
        const ImVec2 ok_button_sz(60, 40);
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ok_button_sz.x) / 2);
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ok_button_sz.y - 15);
        if (ImGui::Button("OK", ok_button_sz)) {
            state_ = SAVED;
        }
        ImGui::End();
    }
    ImGui::PopStyleColor();
}

void LevelEditor::RenderEditor(float game_area_height, float game_area_width) {
    switch (state_) {
        case DRAW_BRUSH:
            RenderDrawBrush(game_area_height, game_area_width);
            break;
        case SAVE_UI:
        case SAVED:
            RenderSaveUI();
            break;
        default:
            break;
    }
}

bool LevelEditor::ProcessInput(int mouse_block_ind_x, int mouse_block_ind_y) {
    // draw if mouse left button down.
    if (mouse_button1_down) {
        GameResource::Position mouse_position(mouse_block_ind_x, mouse_block_ind_y);
        std::set<GameResource::Position>::const_iterator itr;
        switch (selected_source_obj_) {
            case GameResource::WALL:
            case GameResource::FLOOR:
                game_level.game_wall_map[mouse_block_ind_y][mouse_block_ind_x] = selected_source_obj_;
                EraseElemIfExist(box_positions_, mouse_position);
                EraseElemIfExist(destination_positions_, mouse_position);
                (*destination_record_ptr_)[mouse_block_ind_y][mouse_block_ind_x] = false;
                break;
            case GameResource::BOX:
                if (box_positions_.find(mouse_position) == box_positions_.end()) {
                    box_positions_.emplace(mouse_block_ind_x, mouse_block_ind_y);
                }
                game_level.game_wall_map[mouse_block_ind_y][mouse_block_ind_x] = GameResource::FLOOR;
                break;
            case GameResource::DESTINATION:
                if (destination_positions_.find(mouse_position) == destination_positions_.end()) {
                    destination_positions_.emplace(mouse_block_ind_x, mouse_block_ind_y);
                    (*destination_record_ptr_)[mouse_block_ind_y][mouse_block_ind_x] = true;
                }
                if (game_level.game_wall_map[mouse_block_ind_y][mouse_block_ind_x] == GameResource::WALL) {
                    game_level.game_wall_map[mouse_block_ind_y][mouse_block_ind_x] = GameResource::FLOOR;
                }
                break;
            case GameResource::PLAYER:
                game_level.player_position.x = mouse_block_ind_x;
                game_level.player_position.y = mouse_block_ind_y;
                EraseElemIfExist(box_positions_, mouse_position);
                is_player_pos_seted = true;
                game_level.game_wall_map[mouse_block_ind_y][mouse_block_ind_x] = GameResource::FLOOR;
                break;
            default:
                break;
        }
    }
    return true;
}

bool LevelEditor::CheckWallClosed(const std::vector<std::vector<GameResource::ObjectName>> &game_map,
                                  const GameResource::Position &player_position) {
    std::set<GameResource::Position> visited;
    // bfs
    std::queue<GameResource::Position> que;
    auto start_pos = GameResource::Position(player_position.x, player_position.y);
    que.push(start_pos);
    visited.insert(start_pos);
    const int NUM_DIRECTIONS = 4;
    const int direction[NUM_DIRECTIONS][2] = {{0,  -1},
                                              {0,  1},
                                              {-1, 0},
                                              {1,  0}};
    int count_box = 0;
    int count_destination = 0;
    while (!que.empty()) {
        auto cur = que.front();
        que.pop();
        for (int i = 0; i < NUM_DIRECTIONS; ++i) {
            GameResource::Position moved(cur.x + direction[i][0], cur.y + direction[i][1]);
            if (visited.find(moved) != visited.end()) {
                continue;
            }
            if (moved.y < 0 || moved.y >= game_map.size() ||
                moved.x < 0 || moved.x >= game_map[moved.y].size()) {
                return false;
            }
            visited.insert(moved);
            if (game_map[moved.y][moved.x] != GameResource::WALL) {
                que.push(moved);
            }
            if (box_positions_.find(moved) != box_positions_.end()) {
                ++count_box;
            }
            if (destination_positions_.find(moved) != destination_positions_.end()) {
                ++count_destination;
            }
        }
    }
    if (destination_positions_.find(player_position) != destination_positions_.end()) {
        ++count_destination;
    }
    if (count_destination != destination_positions_.size()) {
        hint_str = "There are some destinations cannot be reached by player.";
        return false;
    }
    if (count_box != box_positions_.size()) {
        hint_str = "There are some box cannot be reached by player.";
        return false;
    }
    return true;
}

void BFS_SetOutSide(std::vector<std::vector<GameResource::ObjectName>> &game_map, int start_i, int start_j) {
    if (game_map[start_i][start_j] != GameResource::FLOOR) {
        return;
    }
    std::queue<std::pair<int, int>> que;
    que.push(std::make_pair(start_i, start_j));
    const int NUM_DIRECTION = 4;
    const int direction[NUM_DIRECTION][2] = {{0,  -1},
                                             {0,  1},
                                             {-1, 0},
                                             {1,  0}};
    while (!que.empty()) {
        auto cur = que.front();
        que.pop();
        for (auto dx: direction) {
            auto moved = std::make_pair(cur.first + dx[0], cur.second + dx[1]);
            if (moved.first < 0 || moved.first >= game_map.size() ||
                moved.second < 0 || moved.second >= game_map[moved.first].size()) {
                continue;
            }
            if (game_map[moved.first][moved.second] == GameResource::OUTSIDE ||
                game_map[moved.first][moved.second] == GameResource::WALL) {
                continue;
            }
            game_map[moved.first][moved.second] = GameResource::OUTSIDE;
            que.push(moved);
        }
    }
}

bool LevelEditor::PrunOutsideFloors() {
    // trans FLOOR outside external wall to OUTSIDE.
    for (int i = 0; i < game_level.game_wall_map.size(); ++i) {
        BFS_SetOutSide(game_level.game_wall_map, i, 0);
        BFS_SetOutSide(game_level.game_wall_map, i, game_level.game_wall_map[i].size() - 1);
    }
    for (int j = 0; j < game_level.game_wall_map[0].size(); ++j) {
        BFS_SetOutSide(game_level.game_wall_map, 0, j);
    }
    for (int j = 0; j < game_level.game_wall_map[game_level.game_wall_map.size() - 1].size(); ++j) {
        BFS_SetOutSide(game_level.game_wall_map, game_level.game_wall_map.size() - 1, j);
    }

    int up_index = 0, down_index = game_level.game_wall_map.size() - 1;
    while (up_index < game_level.game_wall_map.size()) {
        if (std::any_of(game_level.game_wall_map[up_index].begin(), game_level.game_wall_map[up_index].end(),
                        [](const GameResource::ObjectName &obj_name) { return obj_name != GameResource::OUTSIDE; })) {
            break;
        }
        ++up_index;
    }
    --up_index;
    while (down_index >= 0) {
        if (std::any_of(game_level.game_wall_map[down_index].begin(), game_level.game_wall_map[down_index].end(),
                        [](const GameResource::ObjectName &obj_name) { return obj_name != GameResource::OUTSIDE; })) {
            break;
        }
        --down_index;
    }
    ++down_index;
    int left_index = map_n_cols, right_index = 0;
    for (auto &row: game_level.game_wall_map) {
        int first_index = 0;
        for (; first_index < row.size(); ++first_index) {
            if (row[first_index] != GameResource::OUTSIDE) {
                break;
            }
        }
        left_index = std::min(left_index, first_index - 1);
        int last_index = row.size() - 1;
        for (; last_index >= 0; --last_index) {
            if (row[last_index] != GameResource::OUTSIDE) {
                break;
            }
        }
        right_index = std::max(right_index, last_index + 1);
    }
    map_n_rows = down_index - up_index - 1;
    map_n_cols = right_index - left_index - 1;
    std::vector<std::vector<GameResource::ObjectName>> prunned_map(map_n_rows);
    for (int i = 0; i < map_n_rows; ++i) {
        prunned_map[i].resize(map_n_cols);
        for (int j = 0; j < map_n_cols; ++j) {
            prunned_map[i][j] = game_level.game_wall_map[i + up_index + 1][j + left_index + 1];
        }
    }
    game_level.game_wall_map = std::move(prunned_map);

    game_level.box_positions.reserve(box_positions_.size());
    for (const auto &box_pos: box_positions_) {
        game_level.box_positions.emplace_back(box_pos.x - left_index - 1, box_pos.y - up_index - 1);
    }
    game_level.destination_positions.reserve(destination_positions_.size());
    for (const auto &dest_pos: destination_positions_) {
        game_level.destination_positions.emplace_back(dest_pos.x - left_index - 1, dest_pos.y - up_index - 1);
    }
    game_level.player_position.x -= left_index + 1;
    game_level.player_position.y -= up_index + 1;
    return true;
}

/**
 *
 * @return true iff saved_ success.
 */
bool LevelEditor::SaveLevelData() {
    state_ = SAVE_UI;
    hint_str.clear();
    if (!is_player_pos_seted) {
        hint_str = "You must set the player station.";
        return false;
    } else if (box_positions_.size() != destination_positions_.size()) {
        hint_str = "The number of boxes and destinations should be equal.";
        return false;
    } else if (game_level.game_wall_map[game_level.player_position.y][game_level.player_position.x] ==
               GameResource::WALL) {
        hint_str = "The player is covered by wall.";
        return false;
    } else if (!CheckWallClosed(game_level.game_wall_map, game_level.player_position)) {
        if (hint_str.size() == 0) {
            hint_str = "The external wall is not closed.";
        }
        return false;
    }

    PrunOutsideFloors();
    game_resource_ptr_->AddLevel(game_level);
    game_resource_ptr_->DumpLevelData(game_resource_ptr_->GetListLevels(), game_resource_ptr_->level_data_file_name);
    saved_ = true;
    return true;
}
