//
// Created by 冒家豪 on 2023/8/27.
//

#ifndef DEMO_IMGUI_GAMERESOURCE_H
#define DEMO_IMGUI_GAMERESOURCE_H

#include "mySDL2Tools/MySDLStruct.h"

#include <string>

class GameResource {
public:
    GameResource(SDL_Renderer *renderer) {
        LoadImageResource(renderer);
    }

    SDL_Texture *GetWallTexture() {
        return wall_texture.texture;
    }

    SDL_Texture *GetFloor1Textuer() {
        return floor1_texture.texture;
    }

    SDL_Texture  *GetFloor2Texture() {
        return floor2_texture.texture;
    }

    SDL_Texture *GetBoxTexture() {
        return box_texture.texture;
    }

    SDL_Texture *GetDestinationTexture() {
        return destination_texture.texture;
    }

    SDL_Texture *GetPlayerTextuer() {
        return player_texture.texture;
    }

private:
    const std::string root_dir = "../src/";
    const std::string img_wall = "wall.png";
    const std::string img_floor1 = "floor1.png";
    const std::string img_floor2 = "floor2.png";
    const std::string img_box = "box.png";
    const std::string img_destination = "destination.png";
    const std::string img_player = "player.png";

    TextureWrap wall_texture, floor1_texture, floor2_texture;
    TextureWrap box_texture, destination_texture, player_texture;

    void LoadImageResource(SDL_Renderer *renderer) {
        wall_texture.createFromIMG(renderer, (root_dir + img_wall).c_str());
        floor1_texture.createFromIMG(renderer, (root_dir + img_floor1).c_str());
        floor2_texture.createFromIMG(renderer, (root_dir + img_floor2).c_str());
        box_texture.createFromIMG(renderer, (root_dir + img_box).c_str());
        destination_texture.createFromIMG(renderer, (root_dir + img_destination).c_str());
        player_texture.createFromIMG(renderer, (root_dir + img_player).c_str());
    }
};


#endif //DEMO_IMGUI_GAMERESOURCE_H
