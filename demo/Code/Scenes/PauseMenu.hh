// SPDX-License-Identifier: MIT
// Copyright (C) Wasym A. Alonso. All Rights Reserved.

struct PauseMenu final : gph::Scene {
  using Scene::Scene;

  virtual void Born(void) override {
    cbn::win::SetMouseVisibility(true);
  }

  virtual void Update([[maybe_unused]] const f64 dt) override {
    if (cbn::win::GetKeyDown(cbn::win::KeyCode::Escape)) {
      r_SceneMgr.Pop();
    }
  }

  virtual void Render(void) const override {
    r_Canvas.Fill(Color::Black);
    r_Canvas.DrawText("PAUSE", cbn::math::Vec2(200), 10, Color::Red);
  }
};
