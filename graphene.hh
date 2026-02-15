/*
  Game engine C++ header-only library. MIT. See license statements at the end of this file.
  graphene - v0.1-alpha - YYYY-MM-DD

  Wasym Atieh Alonso - wasymatieh01@gmail.com

  Website: https://sparky-game.org/software/graphene
  GitHub:  https://github.com/sparky-game/graphene
*/

/*
  1. Introduction
  ===============
  (...)

  2. Building
  ===========
  (...)

  3. Definitions
  ==============
  This section defines common terms used throughout graphene.

  3.1. Entity
  -----------
  (...)

  3.2. Scene
  ----------
  (...)

  3.2.1. Callback execution lifecycle
  -----------------------------------
  (...)

  - Born() :: upon creation
  - Die() :: upon destruction
  - Awake() :: upon enable/resume
  - Snooze() :: upon disable/pause

  4. Folder structure suggestion
  ==============================
  Assets
  +---Art
  | +---Sprites
  | +---Meshes
  | \---Fonts
  +---Audio
  | +---Music
  | \---Sound
  Code
  +---Data
  +---Entities
  \---Scenes
*/

// SPDX-License-Identifier: MIT
// Copyright (C) Wasym A. Alonso. All Rights Reserved.

#pragma once

#include <carbon.h>

#if CARBON_VERSION_MAJOR != 0 || CARBON_VERSION_MINOR != 17
#error Carbon v0.17 is needed
#endif

namespace gph {
  /**
   * @brief ...
   */
  struct Entity {
    explicit Entity(void) = default;
    Entity(const Entity &) = delete;
    Entity(Entity &&) = delete;
    Entity &operator=(const Entity &) = delete;
    Entity &operator=(Entity &&) = delete;
    virtual ~Entity(void) = default;
    virtual void Update([[maybe_unused]] const f64 dt) {}
    virtual void Render([[maybe_unused]] cbn::DrawCanvas &dc) const {}
  };

  /**
   * @brief ...
   */
  // TODO: migrate to `cbn::meta::IsBase_v` and `cbn::meta::IsConvertible_v`
  template <typename T>
  concept ValidEntity = std::is_base_of_v<Entity, T> and std::is_convertible_v<T*, Entity*>;

  /**
   * @brief ...
   */
  namespace RenderLayer {
    enum T {
      Background,
      Foreground,
      UI,
      Count
    };
    template <T Layer>
    concept Valid = Layer != Count;
  };

  struct SceneManager;  // Forward declaration
  struct AssetManager;  // Forward declaration

  /**
   * @brief ...
   */
  struct Scene {
    explicit Scene(cbn::DrawCanvas &dc, SceneManager &s_mgr, const AssetManager &a_mgr)
      : r_Canvas{dc},
        r_SceneMgr{s_mgr},
        r_AssetMgr{a_mgr}
    {
      for (auto &l : m_Entities) {
        l = typeof(m_Entities)::value_type::make();
      }
    }

    Scene(const Scene &) = delete;
    Scene(Scene &&) = delete;
    Scene &operator=(const Scene &) = delete;
    Scene &operator=(Scene &&) = delete;

    virtual ~Scene(void) {
      for (auto &l : m_Entities) {
        for (auto e : l) delete e;
        l.Free();
      }
    }

    virtual void Born(void) {}
    virtual void Die(void) {}
    virtual void Awake(void) {}
    virtual void Snooze(void) {}
    virtual void Update([[maybe_unused]] const f64 dt) {}
    virtual void Render(void) const {}

    void UpdateAll(const f64 dt) {
      Update(dt);
      for (const auto &l : m_Entities) {
        for (auto e : l) e->Update(dt);
      }
    }

    void RenderAll(void) const {
      Render();
      for (const auto &l : m_Entities) {
        for (auto e : l) e->Render(r_Canvas);
      }
    }

  protected:
    cbn::DrawCanvas &r_Canvas;
    SceneManager &r_SceneMgr;
    const AssetManager &r_AssetMgr;

    template <ValidEntity E, RenderLayer::T L, typename... Args>
    requires RenderLayer::Valid<L>
    E &NewEntity(Args &&... args) {
      auto e = new E{cbn::meta::Forward<Args>(args)...};
      m_Entities[L].Push(e);
      return *e;
    }

  private:
    std::array<cbn::List<Entity *>, RenderLayer::Count> m_Entities;
  };

  /**
   * @brief ...
   */
  // TODO: migrate to `cbn::meta::IsBase_v` and `cbn::meta::IsConvertible_v`
  template <typename T>
  concept ValidScene = std::is_base_of_v<Scene, T> and std::is_convertible_v<T*, Scene*>;

  /**
   * @brief ...
   */
  struct SceneManager final {
    explicit SceneManager(void) : m_Scenes{typeof(m_Scenes)::make()} {}

    SceneManager(const SceneManager &) = delete;
    SceneManager(SceneManager &&) = delete;
    SceneManager &operator=(const SceneManager &) = delete;
    SceneManager &operator=(SceneManager &&) = delete;

    ~SceneManager(void) {
      while (m_Scenes.size) Pop();
      m_Scenes.Free();
    }

    usz Count(void) const {
      return m_Scenes.size;
    }

    template <ValidScene S>
    void Switch(cbn::DrawCanvas &dc, const AssetManager &a_mgr) {
      while (m_Scenes.size) Pop();
      Push<S>(dc, a_mgr);
    }

    template <ValidScene S>
    void Push(cbn::DrawCanvas &dc, const AssetManager &a_mgr) {
      auto ns = new S{dc, *this, a_mgr};
      if (m_Scenes.size) m_Scenes.Back()->Snooze();
      m_Scenes.Push(ns);
      ns->Born();
    }

    void Pop(void) {
      if (!m_Scenes.size) return;
      auto curr = m_Scenes.Back();
      curr->Die();
      delete curr;
      m_Scenes.PopBack();
      if (m_Scenes.size) m_Scenes.Back()->Awake();
    }

    void Update(const f64 dt) const {
      if (m_Scenes.size) m_Scenes.Back()->UpdateAll(dt);
    }

    void Render(void) const {
      if (m_Scenes.size) m_Scenes.Back()->RenderAll();
    }

  private:
    cbn::List<Scene *> m_Scenes;
  };

  /**
   * @brief ...
   */
  struct AssetManager final {
    explicit AssetManager(const char *ap_path)
      : m_AssetPack{cbn::SKAP::make(ap_path)}
    {
      if (!m_AssetPack) CARBON_UNREACHABLE;
      cbn::sprite_mgr::Init();
      cbn::audio::Init();
    }

    AssetManager(const AssetManager &) = delete;
    AssetManager(AssetManager &&) = delete;
    AssetManager &operator=(const AssetManager &) = delete;
    AssetManager &operator=(AssetManager &&) = delete;

    ~AssetManager(void) {
      cbn::audio::Shutdown();
      cbn::sprite_mgr::Shutdown();
      m_AssetPack->Free();
    }

    cbn::sprite_mgr::UID LoadSprite(const char *name) const {
      auto s = m_AssetPack->LoadSprite(name);
      if (!s) CARBON_UNREACHABLE;
      return *s;
    }

  private:
    cbn::Opt<cbn::SKAP> m_AssetPack;
  };

  /**
   * @brief ...
   */
  struct Game final {
    struct Spec {
      usz width, height;
      const char *title;
      const char *ap_path;
    };

    explicit Game(const Spec &s)
      : m_Canvas{cbn::DrawCanvas::New(s.width, s.height)},
        m_AssetMgr{s.ap_path}
    {
      m_Canvas->OpenWindow(s.title);
    }

    Game(const Game &) = delete;
    Game(Game &&) = delete;
    Game &operator=(const Game &) = delete;
    Game &operator=(Game &&) = delete;

    ~Game(void) {
      cbn::win::Close();
    }

    template <ValidScene S>
    void InitScene(void) {
      m_SceneMgr.Switch<S>(*m_Canvas, m_AssetMgr);
    }

    void Run(void) {
      cbn::win::ForFrame([this](const auto dt){
        m_SceneMgr.Update(dt);
        m_SceneMgr.Render();
        m_Canvas->UpdateWindow();
      });
    }

  private:
    cbn::Scope<cbn::DrawCanvas> m_Canvas;
    SceneManager m_SceneMgr;
    AssetManager m_AssetMgr;
  };
}

/*
  This software is availabe under the MIT license:
  ================================================
  Copyright (C) Wasym A. Alonso

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
