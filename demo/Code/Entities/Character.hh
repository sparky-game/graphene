template <Color C, usz S>
struct Character : gph::Entity {
  static constexpr auto c_Size {S};
  cbn::math::Vec2 m_Position {};
  f32 m_Orientation {};

  explicit Character(const cbn::math::Vec2 &p = {}) : m_Position{p} {}

  virtual void Render(cbn::DrawCanvas &dc) const override {
    static constexpr auto thickness = 5;
    {// Square w/outline
      static constexpr auto in_color = (u32)C;
      static constexpr auto out_color = (u32)Color::White;
      dc.DrawRect(cbn::math::Rect(m_Position - c_Size/2, c_Size), out_color);
      dc.DrawRect(cbn::math::Rect(m_Position - c_Size/2 + thickness, c_Size - 2*thickness), in_color);
    }
    {// Orientation indicator
      static constexpr auto color = (u32)Color::Red;
      static constexpr auto s = 1.5 * thickness;
      const auto v_phi = (c_Size/2) * cbn::math::Vec2{
        cbn::math::Cos(m_Orientation),
        cbn::math::Sin(m_Orientation)
      };
      dc.DrawRect(cbn::math::Rect(m_Position - s/2 + v_phi, s), color);
    }
  }
};
