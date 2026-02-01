struct Player final : Character<Color::Blue, 50> {
  const Crosshair &r_Crosshair;

  explicit Player(const Crosshair &ch, const cbn::math::Vec2 &p = {})
    : Character{p},
      r_Crosshair{ch}
  {}

  virtual void Update(const f64 dt) override {
    {// Position
      static constexpr auto speed = 300;
      cbn::math::Vec2 dir {
        (f32) cbn::win::GetKey(cbn::win::KeyCode::D) - cbn::win::GetKey(cbn::win::KeyCode::A),
        (f32) cbn::win::GetKey(cbn::win::KeyCode::S) - cbn::win::GetKey(cbn::win::KeyCode::W)
      };
      m_Position += dir.Normalize() * speed * dt;
    }
    {// Orientation
      auto v = r_Crosshair.m_Position - m_Position;
      m_Orientation = cbn::math::Atan2(v.y, v.x);
    }
  }
};
