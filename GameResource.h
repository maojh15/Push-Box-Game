//
// Created by 冒家豪 on 2023/8/27.
//

#ifndef DEMO_IMGUI_GAMERESOURCE_H
#define DEMO_IMGUI_GAMERESOURCE_H

#include "LoadTexture.h"
#include "imgui.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class GameResource {
public:
    GameResource() {
        std::cout << "Initialized Game resource ... " << std::endl;
        ch_to_obj_name['w'] = ch_to_obj_name['W'] = WALL;
        ch_to_obj_name['f'] = ch_to_obj_name['F'] = FLOOR;
        ch_to_obj_name['b'] = ch_to_obj_name['B'] = BOX;
        ch_to_obj_name['d'] = ch_to_obj_name['D'] = DESTINATION;
        ch_to_obj_name['p'] = ch_to_obj_name['P'] = PLAYER;
        LoadImageResource();
        LoadLevelResource();
        std::cout << "Initialization Finished" << std::endl;
    }

    GLuint GetWallTexture() {
        return wall_texture->textureID;
    }

    GLuint GetFloorTexture() {
        return floor_texture->textureID;
    }

    GLuint GetBoxTexture() {
        return box_texture->textureID;
    }

    GLuint GetDestinationTexture() {
        return destination_texture->textureID;
    }

    GLuint GetPlayerTextuer() {
        return player_texture->textureID;
    }

    const std::shared_ptr<LoadTextureTool> GetBackgroundTextureObj() {
        return background_texture;
    }

    const std::shared_ptr<LoadTextureTool> GetWinTextureObj() {
        return win_texture;
    }

    enum ObjectName {
        WALL, FLOOR, BOX, DESTINATION, PLAYER, OUTSIDE
    };

    std::vector<std::vector<ObjectName>> level_data;
    std::vector<std::vector<bool>> destination_record;
    int map_num_rows, map_num_cols;

    struct Position {
        int x, y;

        Position(int x = 0, int y = 0) : x{x}, y{y} {}
    };

    ImVec2 player_face_uv0{0, 0};
    ImVec2 player_face_uv1{0.5, 1};
    ImVec2 player_run_uv0{0.5, 0};
    ImVec2 player_run_uv1{1, 1};
    Position initial_player_position{0, 0};
    std::vector<Position> initial_box_positions;
    std::vector<Position> initial_destination_positions;
private:
    const std::string root_dir = "./src/";
    const std::string img_wall = "wall.png";
    const std::string img_floor = "floor.png";
    const std::string img_box = "box.png";
    const std::string img_destination = "destination.png";
    const std::string img_player = "player.png";

    const std::string img_background = "background.jpg";
    const std::string img_win = "Win.png";

    std::shared_ptr<LoadTextureTool> wall_texture, floor_texture;
    std::shared_ptr<LoadTextureTool> box_texture, destination_texture, player_texture;
    std::shared_ptr<LoadTextureTool> background_texture, win_texture;

    const std::string text_level = "level.txt";
    std::unordered_map<char, ObjectName> ch_to_obj_name;

    void LoadImageResource() {
        wall_texture = std::make_shared<LoadTextureTool>((root_dir + img_wall).c_str());
        floor_texture = std::make_shared<LoadTextureTool>((root_dir + img_floor).c_str());
        box_texture = std::make_shared<LoadTextureTool>((root_dir + img_box).c_str());
        destination_texture = std::make_shared<LoadTextureTool>((root_dir + img_destination).c_str());
        player_texture = std::make_shared<LoadTextureTool>((root_dir + img_player).c_str());
        background_texture = std::make_shared<LoadTextureTool>((root_dir + img_background).c_str());
        win_texture = std::make_shared<LoadTextureTool>((root_dir + img_win).c_str());
    }

    void LoadLevelResource() {
        std::ifstream level_infile{root_dir + text_level};
        if (!level_infile.is_open()) {
            throw std::runtime_error("Cannot open file " + text_level);
        }

        std::string line;
        map_num_rows = 0;
        map_num_cols = 0;
        int pos_y = 0;
        while (std::getline(level_infile, line)) {
            std::vector<ObjectName> object_name_list;
            std::vector<bool> line_destination_record;
            int count_cols = 0;
            int pos_x = 0;
            for (auto ch: line) {
                if (ch_to_obj_name.find(ch) == ch_to_obj_name.end()) {
                    ch_to_obj_name[ch] = OUTSIDE;
                }
                object_name_list.push_back(ch_to_obj_name[ch]);
                bool is_destination = false;
                switch (ch_to_obj_name[ch]) {
                    case PLAYER:
                        initial_player_position.x = pos_x;
                        initial_player_position.y = pos_y;
                        break;
                    case BOX:
                        initial_box_positions.emplace_back(pos_x, pos_y);
                        break;
                    case DESTINATION:
                        initial_destination_positions.emplace_back(pos_x, pos_y);
                        *(object_name_list.rbegin()) = FLOOR;
                        is_destination = true;
                        break;
                }
                line_destination_record.push_back(is_destination);
                ++count_cols;
                ++pos_x;
            }
            level_data.emplace_back(std::move(object_name_list));
            destination_record.emplace_back(std::move(line_destination_record));
            map_num_cols = std::max(map_num_cols, count_cols);
            ++map_num_rows;
            ++pos_y;
        }
    }
};


#endif //DEMO_IMGUI_GAMERESOURCE_H
