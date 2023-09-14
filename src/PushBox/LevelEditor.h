//
// Created by 冒家豪 on 2023/9/13.
//

#ifndef DEMO_IMGUI_LEVELEDITOR_H
#define DEMO_IMGUI_LEVELEDITOR_H

#include "GameResource.h"

#include <vector>
#include <set>

class LevelEditor {
public:
    enum LevelEditorState {
        SET_MAP_SIZE, DRAW_BRUSH, SAVE_UI, SAVED
    };

    LevelEditor() {
        ResetState();
    }

    void SetGameResourcePtr(GameResource &game_resource) {
        game_resource_ptr_ = &game_resource;
    }

    void ResetState();

    void RenderSetMapSize(int &map_num_rows, int &map_num_cols);

    bool GetShowPlayer() const {
        return is_player_pos_seted;
    }

    bool ShouldRenderGameMap() const {
        return assigned_map_size;
    }

    void SetMousePosition(int mouse_x, int mouse_y) {
        mouse_x_ = mouse_x;
        mouse_y_ = mouse_y;
    }

    void SetDrawParameters(float top_left_pos_x, float top_left_pos_y, float block_len) {
        top_left_pos_x_ = top_left_pos_x;
        top_left_pos_y_ = top_left_pos_y;
        block_len_ = block_len;
    }

    void RenderEditor(float game_area_height, float game_area_width);

    bool SaveLevelData();

    GameResource::GameLevelData game_level;
    bool mouse_button1_down = false;
    std::set<GameResource::Position> box_positions_, destination_positions_;
    std::vector<std::vector<bool>> *destination_record_ptr_;
    LevelEditorState state_;
    bool assigned_map_size = false;
    bool saved_ = false;
    int save_level_id = -1;
private:
    void RenderDrawBrush(float game_area_heigh, float game_area_with);

    void RenderSaveUI();

    bool ProcessInput(int mouse_block_ind_x, int mouse_block_ind_y);

    bool PrunOutsideFloors();

    bool CheckWallClosed(const std::vector<std::vector<GameResource::ObjectName>> &game_map,
                         const GameResource::Position &player_position);

    int map_n_rows = 10, map_n_cols = 10;
    bool is_player_pos_seted = false;
    int mouse_x_, mouse_y_;
    float top_left_pos_x_, top_left_pos_y_;
    float block_len_;
    GameResource *game_resource_ptr_;
    GameResource::ObjectName selected_source_obj_ = GameResource::OUTSIDE;
    std::string hint_str;
    bool save_pressed_flag = false;
};


#endif //DEMO_IMGUI_LEVELEDITOR_H
