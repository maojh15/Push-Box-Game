#include "GameResource.h"

#include <fstream>

/**
 * Dump game level data.
 * @param game_level_data
 * @param dump_data_file
 */
void GameResource::DumpLevelData(const std::vector<GameLevelData> &game_level_data,
                                 const std::string &dump_data_file_name) {
    std::ofstream dump_file{dump_data_file_name, std::ios::out};
    dump_file << game_level_data.size() << "\n";
    for (int level_id = 0; level_id < game_level_data.size(); ++level_id) {
        dump_file << level_id << "\n";
        const auto &level_data = game_level_data[level_id];
        // dump game wall map
        int height = level_data.game_wall_map.size();
        dump_file << height << " ";
        for (int i = 0; i < height; ++i) {
            int sz = level_data.game_wall_map[i].size();
            dump_file << sz << " ";
            for (int j = 0; j < sz; ++j) {
                dump_file << level_data.game_wall_map[i][j] << " ";
            }
        }

        // dump destination positions.
        int sz = level_data.destination_positions.size();
        dump_file << sz << " ";
        for (int i = 0; i < sz; ++i) {
            dump_file << level_data.destination_positions[i].x << " "
                      << level_data.destination_positions[i].y << " ";
        }

        // dump box position.
        sz = level_data.box_positions.size();
        dump_file << sz << " ";
        for (int i = 0; i < sz; ++i) {
            dump_file << level_data.box_positions[i].x << " "
                      << level_data.box_positions[i].y << " ";
        }

        // dump player position.
        dump_file << level_data.player_position.x << " " << level_data.player_position.y << "\n";
    }
}

void GameResource::ReadLevelDataFromFile(std::vector<GameLevelData> &game_level_data,
                                         const std::string &dump_data_file_name) {
    game_level_data.clear();
    std::ifstream fin{dump_data_file_name, std::ios::in};
    int num_levels;
    fin >> num_levels;
    game_level_data.resize(num_levels);
    for (int index = 0; index < num_levels; ++index) {
        int level_id;
        fin >> level_id;
        GameLevelData level_data;

        // read game wall map
        int height;
        fin >> height;
        level_data.game_wall_map.resize(height);
        for (int i = 0; i < height; ++i) {
            int width;
            fin >> width;
            level_data.game_wall_map[i].resize(width);
            for (int j = 0; j < width; ++j) {
                int tmp;
                fin >> tmp;
                level_data.game_wall_map[i][j] = static_cast<ObjectName>(tmp);
            }
        }

        //read destination positions
        int sz;
        fin >> sz;
        level_data.destination_positions.resize(sz);
        for (int i = 0; i < sz; ++i) {
            fin >> level_data.destination_positions[i].x
                >> level_data.destination_positions[i].y;
        }

        // read box position
        fin >> sz;
        level_data.box_positions.resize(sz);
        for (int i = 0; i < sz; ++i) {
            fin >> level_data.box_positions[i].x
                >> level_data.box_positions[i].y;
        }

        // read player position
        fin >> level_data.player_position.x >> level_data.player_position.y;
        game_level_data[level_id] = std::move(level_data);
    }
}