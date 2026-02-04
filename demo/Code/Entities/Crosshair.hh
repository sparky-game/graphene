// SPDX-License-Identifier: MIT
// Copyright (C) Wasym A. Alonso. All Rights Reserved.

struct Crosshair final : gph::Entity {
  const cbn::sprite_mgr::UID m_Sprite;
  cbn::math::Vec2 m_Position {};

  explicit Crosshair(cbn::sprite_mgr::UID s) : m_Sprite{s} {}

  virtual void Update([[maybe_unused]] const f64 dt) override {
    m_Position = cbn::win::GetMousePosition();
  }

  virtual void Render(cbn::DrawCanvas &dc) const override {
    const auto * const sp = cbn::sprite_mgr::Lookup(m_Sprite);
    const auto sp_pos = m_Position - cbn::math::Vec2(sp->width, sp->height)/2;
    dc.DrawSprite(sp, sp_pos);
  }
};
