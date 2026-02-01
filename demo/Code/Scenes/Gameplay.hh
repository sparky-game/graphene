struct Gameplay final : gph::Scene {
  using Scene::Scene;

  virtual void OnEnter(void) override {
    // NOTE: the mouse isn't confined to the window yet, and hidding it makes it
    //       complicated to control when moved outside the window (remains hidden).
    // cbn::win::SetMouseVisibility(false);

    auto s_ch = r_AssetMgr.LoadSprite("Art/Sprites/crosshair.png");
    {// Entities
      auto &e_ch = NewEntity<Crosshair, gph::RenderLayer::UI>(s_ch);
      NewEntity<Player, gph::RenderLayer::Foreground>(e_ch, cbn::math::Vec2(200));
      NewEntity<Enemy, gph::RenderLayer::Foreground>();
    }
  }

  virtual void Update([[maybe_unused]] const f64 dt) override {
    if (cbn::win::GetKeyDown(cbn::win::KeyCode::Escape)) cbn::win::Exit();
  }

  virtual void Render(void) const override {
    r_Canvas.Fill((u32)Color::Black);
  }
};
