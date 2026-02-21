// SPDX-License-Identifier: MIT
// Copyright (C) Wasym A. Alonso. All Rights Reserved.

struct Gameplay final : gph::Scene {
  using Scene::Scene;

  virtual void Born(void) override {
    Awake();
    auto s_ch = r_AssetMgr.LoadSprite("Art/Sprites/crosshair.png");
    auto &e_ch = NewEntity<Crosshair, gph::RenderLayer::UI>(s_ch);
    NewEntity<Player, gph::RenderLayer::Foreground>(e_ch, cbn::math::Vec2(200));
    NewEntity<Enemy, gph::RenderLayer::Foreground>();
  }

  virtual void Awake(void) override {
    cbn::win::SetMouseVisibility(false);
  }

  virtual void Update([[maybe_unused]] const f64 dt) override {
    if (cbn::win::GetKeyDown(cbn::win::KeyCode::Escape)) {
      r_SceneMgr.Push<PauseMenu>(r_Canvas, r_AssetMgr);
    }
  }

  virtual void Render(void) const override {
    r_Canvas.Fill(Color::Black);
  }
};
