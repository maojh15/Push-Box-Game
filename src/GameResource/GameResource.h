//
// Created by 冒家豪 on 2023/8/27.
//

#ifndef DEMO_IMGUI_GAMERESOURCE_H
#define DEMO_IMGUI_GAMERESOURCE_H

#include "LoadTexture.h"
#include "imgui.h"
#include "SDL_filesystem.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class GameResource {
public:
    enum ObjectName {
        WALL, FLOOR, BOX, DESTINATION, PLAYER,
        OUTSIDE // Unreachable area out side walls.
    };

    struct Position {
        int x, y;

        Position(int x = 0, int y = 0) : x{x}, y{y} {}

        bool operator<(const Position &rhs) const {
            return x < rhs.x || (x == rhs.x && y < rhs.y);
        }
    };

    struct GameLevelData {
        std::vector<std::vector<ObjectName>> game_wall_map; // game map. Include WALL, FLOOR, OUTSIDE.
        std::vector<Position> destination_positions; // list of destination positions.
        std::vector<Position> box_positions; // list box positions at beginning.
        Position player_position; // player's position at beginning.

        void Clear() {
            game_wall_map.clear();
            destination_positions.clear();
            box_positions.clear();
            player_position = Position(0, 0);
        }
    };

    GameResource() {
        auto base_path = SDL_GetBasePath();
        root_dir = std::string(base_path) + root_dir;
        level_data_file_name = root_dir + level_data_file_name;
        std::cout << "Initialized Game resource ... " << std::endl;
        LoadImageResource();
        ReadLevelDataFromFile(game_levels, level_data_file_name);
        std::cout << "Initialization Finished" << std::endl;
    }

    GLuint GetWallTexture() const {
        return wall_texture->textureID;
    }

    GLuint GetFloorTexture() const {
        return floor_texture->textureID;
    }

    GLuint GetBoxTexture() const {
        return box_texture->textureID;
    }

    GLuint GetBoxArrivedTexture() const {
        return box_arrived_texture->textureID;
    }

    GLuint GetDestinationTexture() const {
        return destination_texture->textureID;
    }

    GLuint GetPlayerTextuer() const {
        return player_texture->textureID;
    }

    std::shared_ptr<LoadTextureTool> GetBackgroundTextureObj() const {
        return background_texture;
    }

    std::shared_ptr<LoadTextureTool> GetWinTextureObj() const {
        return win_texture;
    }

    ImVec2 player_face_uv0{0, 0};
    ImVec2 player_face_uv1{0.5, 1};
    ImVec2 player_run_uv0{0.5, 0};
    ImVec2 player_run_uv1{1, 1};

    static void DumpLevelData(const std::vector<GameLevelData> &game_level_data,
                              const std::string &dump_data_file_name);

    static void ReadLevelDataFromFile(std::vector<GameLevelData> &game_level_data,
                                      const std::string &dump_data_file_name);

    GameLevelData &GetLevelData(int level_id) {
        return game_levels[level_id];
    }

    bool IsLevelIdExist(int level_id) const {
        return level_id >= 0 && level_id < game_levels.size();
    }

    void AddLevel(const GameLevelData &level_data) {
        game_levels.push_back(level_data);
    }

    auto GetListLevels() const {
        return game_levels;
    }

    std::string level_data_file_name = "level_data";

private:
    std::string root_dir = "Resources/";
    const std::string img_wall = "wall.png";
    const std::string img_floor = "floor.png";
    const std::string img_box = "box.png";
    const std::string img_box_arrived = "box2.png";
    const std::string img_destination = "destination.png";
    const std::string img_player = "player.png";

    const std::string img_background = "background.jpg";
    const std::string img_win = "Win.png";

    std::shared_ptr<LoadTextureTool> wall_texture, floor_texture;
    std::shared_ptr<LoadTextureTool> box_texture, destination_texture, player_texture;
    std::shared_ptr<LoadTextureTool> box_arrived_texture;
    std::shared_ptr<LoadTextureTool> background_texture, win_texture;

    void LoadImageResource() {
        wall_texture = std::make_shared<LoadTextureTool>((root_dir + img_wall).c_str());
        floor_texture = std::make_shared<LoadTextureTool>((root_dir + img_floor).c_str());
        box_texture = std::make_shared<LoadTextureTool>((root_dir + img_box).c_str());
        box_arrived_texture = std::make_shared<LoadTextureTool>((root_dir + img_box_arrived).c_str());
        destination_texture = std::make_shared<LoadTextureTool>((root_dir + img_destination).c_str());
        player_texture = std::make_shared<LoadTextureTool>((root_dir + img_player).c_str());
        background_texture = std::make_shared<LoadTextureTool>((root_dir + img_background).c_str());
        win_texture = std::make_shared<LoadTextureTool>((root_dir + img_win).c_str());
    }

    std::vector<GameLevelData> game_levels; // map from level_id to game_levels.
};


#endif //DEMO_IMGUI_GAMERESOURCE_H
