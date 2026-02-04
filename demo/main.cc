// SPDX-License-Identifier: MIT
// Copyright (C) Wasym A. Alonso. All Rights Reserved.

#include <graphene.hh>
// Data
#include "Code/Data/Color.hh"
// Entities
#include "Code/Entities/Crosshair.hh"
#include "Code/Entities/Character.hh"
#include "Code/Entities/Player.hh"
#include "Code/Entities/Enemy.hh"
// Scenes
#include "Code/Scenes/PauseMenu.hh"
#include "Code/Scenes/Gameplay.hh"
#include "Code/Scenes/MainMenu.hh"

int main(void) {
  gph::Game::Spec spec {
    .width = 1280, .height = 720,
    .title = "Le Game™",
    .ap_path = "Assets/assets.skap"
  };
  gph::Game game {spec};
  game.InitScene<MainMenu>();
  game.Run();
}
