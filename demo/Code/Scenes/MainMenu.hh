struct MainMenu final : gph::Scene {
  using Scene::Scene;

  virtual void Update([[maybe_unused]] const f64 dt) override {
    if (cbn::win::GetKeyDown(cbn::win::KeyCode::One)) {
      r_SceneMgr.Switch<Gameplay>(r_Canvas, r_AssetMgr);
    }
    else if (cbn::win::GetKeyDown(cbn::win::KeyCode::Two)) {
      cbn::win::Exit();
    }
  }

  virtual void Render(void) const override {
    r_Canvas.Fill((u32)Color::Black);
    Render_Title<"Le Game", 100>();
    Render_Button<"<1> Play", 300>();
    Render_Button<"<2> Exit", 500>();
  }

private:
  template <cbn::meta::String Text, usz YPos>
  void Render_Title(void) const {
    static constexpr auto text = Text.value;
    static constexpr auto text_size = 14;
    static constexpr auto text_color = (u32)Color::White;
    static const auto text_width = r_Canvas.TextWidth(text, text_size);
    static const auto text_pos = cbn::math::Vec2(r_Canvas.width/2 - text_width/2, YPos);
    r_Canvas.DrawText(text, text_pos, text_size, text_color);
  }

  template <cbn::meta::String Text, usz YPos>
  void Render_Button(void) const {
    static constexpr auto text = Text.value;
    static constexpr auto text_size = 4;
    static constexpr auto text_color = (u32)Color::Black;
    static constexpr cbn::math::Vec2 text_padding {20, 15};
    static const auto text_width = r_Canvas.TextWidth(text, text_size);
    static const auto text_height = r_Canvas.TextHeight(text_size);
    static const auto text_pos = cbn::math::Vec2(r_Canvas.width/2 - text_width/2, YPos);
    static constexpr auto btn_color = (u32)Color::White;
    static const cbn::math::Rect btn_xywh {
      text_pos.x - text_padding.x,
      text_pos.y - text_padding.y - 2*text_size,
      text_width + 2*text_padding.x,
      text_height + 2*text_padding.y + 2*text_size
    };
    r_Canvas.DrawRect(btn_xywh, btn_color);
    r_Canvas.DrawText(text, text_pos, text_size, text_color);
  }
};
