// SPDX-License-Identifier: MIT
// Copyright (C) Wasym A. Alonso. All Rights Reserved.

template <u32 C, usz S>
struct Character : gph::Entity {
  static constexpr auto c_Color {C};
  static constexpr auto c_Size {S};
  cbn::math::Vec2 m_Position {};
  f32 m_Orientation {};

  explicit Character(const cbn::math::Vec2 &p = {}) : m_Position{p} {}

  virtual void Render(cbn::DrawCanvas &dc) const override {
    static constexpr auto thickness = 5;
    {// Square w/outline
      static constexpr auto outline = Color::White;
      dc.DrawRect(cbn::math::Rect(m_Position - c_Size/2, c_Size), outline);
      dc.DrawRect(cbn::math::Rect(m_Position - c_Size/2 + thickness, c_Size - 2*thickness), c_Color);
    }
    {// Orientation indicator
      static constexpr auto color = Color::Red;
      static constexpr auto s = 1.5 * thickness;
      const auto v_phi = (c_Size/2) * cbn::math::Vec2{
        cbn::math::Cos(m_Orientation),
        cbn::math::Sin(m_Orientation)
      };
      dc.DrawRect(cbn::math::Rect(m_Position - s/2 + v_phi, s), color);
    }
  }
};
