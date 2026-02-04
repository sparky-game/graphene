// SPDX-License-Identifier: MIT
// Copyright (C) Wasym A. Alonso. All Rights Reserved.

struct Enemy final : Character<Color::Orange, 40> {
  virtual void Update(const f64 dt) override {
    m_Position += 30 * dt;
  }
};
