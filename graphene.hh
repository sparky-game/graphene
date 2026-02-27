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

  2.1. Build options
  ------------------
  `#define` these options before including this header, or pass them as compiler flags:

  +-----------------------+--------------------------------+
  | Option                | Description                    |
  +-----------------------+--------------------------------+
  | GPH_ASSETPACK         | Specifies the relative path to |
  |                       | the SKAP AssetPack to use.     |
  |                       | By default it's set to         |
  |                       | "Assets.skap".                 |
  +-----------------------+--------------------------------+

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

  Scene:
  - Born() :: upon creation
  - Die() :: upon destruction
  - Awake() :: upon resume
  - Snooze() :: upon pause

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

#if CARBON_VERSION_MAJOR != 0 || CARBON_VERSION_MINOR != 18
#error Carbon v0.18 is needed
#endif

#ifndef GPH_ASSETPACK
#define GPH_ASSETPACK "Assets.skap"
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
  template <typename T>
  concept ValidEntity = cbn::meta::IsConvertible_v<T*, Entity*>;

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

  /**
   * @brief ...
   */
  struct EntityPool final {
    explicit EntityPool(void) {
      for (auto &l : m_Entities) {
        l = typeof(m_Entities)::value_type::make();
      }
    }

    ~EntityPool(void) {
      for (auto &l : m_Entities) {
        for (auto e : l) delete e;
        l.Free();
      }
    }

    template <ValidEntity E, RenderLayer::T L, typename... Args>
    requires RenderLayer::Valid<L>
    E &Push(Args &&... args) {
      auto e = new E{cbn::meta::Forward<Args>(args)...};
      m_Entities[L].Push(e);
      return *e;
    }

    void Update(const f64 dt) {
      for (const auto &l : m_Entities) {
        for (auto e : l) e->Update(dt);
      }
    }

    void Render(cbn::DrawCanvas &dc) const {
      for (const auto &l : m_Entities) {
        for (auto e : l) e->Render(dc);
      }
    }

  private:
    std::array<cbn::List<Entity *>, RenderLayer::Count> m_Entities;
  };

  /**
   * @brief ...
   */
  struct Scene {
    explicit Scene(cbn::DrawCanvas &dc, SceneManager &s_mgr)
      : r_Canvas{dc}, r_SceneMgr{s_mgr}
    {}

    Scene(const Scene &) = delete;
    Scene(Scene &&) = delete;
    Scene &operator=(const Scene &) = delete;
    Scene &operator=(Scene &&) = delete;

    virtual ~Scene(void) = default;

    virtual void Born(void) {}
    virtual void Die(void) {}
    virtual void Awake(void) {}
    virtual void Snooze(void) {}
    virtual void Update([[maybe_unused]] const f64 dt) {}
    virtual void Render(void) const {}

    void UpdateAll(const f64 dt) {
      Update(dt);
      m_Pool.Update(dt);
    }

    void RenderAll(void) const {
      Render();
      m_Pool.Render(r_Canvas);
    }

  protected:
    cbn::DrawCanvas &r_Canvas;
    SceneManager &r_SceneMgr;

    template <ValidEntity E, RenderLayer::T L, typename... Args>
    requires RenderLayer::Valid<L>
    E &NewEntity(Args &&... args) {
      return m_Pool.Push<E, L>(cbn::meta::Forward<Args>(args)...);
    }

  private:
    EntityPool m_Pool;
  };

  /**
   * @brief ...
   */
  template <typename T>
  concept ValidScene = cbn::meta::IsConvertible_v<T*, Scene*>;

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
      Drain();
      m_Scenes.Free();
    }

    usz Count(void) const {
      return m_Scenes.size;
    }

    template <ValidScene S>
    void Switch(cbn::DrawCanvas &dc) {
      Drain();
      Push<S>(dc);
    }

    template <ValidScene S>
    void Push(cbn::DrawCanvas &dc) {
      auto ns = new S{dc, *this};
      if (m_Scenes.size) m_Scenes.Back()->Snooze();
      m_Scenes.Push(ns);
      ns->Born();
    }

    void Pop(void) {
      m_PendingPop = true;
    }

    void Update(const f64 dt) {
      if (m_Scenes.size) m_Scenes.Back()->UpdateAll(dt);
      FlushDeferred();
    }

    void Render(void) const {
      if (m_Scenes.size) m_Scenes.Back()->RenderAll();
    }

  private:
    void Drain(void) {
      while (m_Scenes.size) PopNow();
    }

    void PopNow(void) {
      if (!m_Scenes.size) return;
      auto curr = m_Scenes.Back();
      curr->Die();
      delete curr;
      m_Scenes.PopBack();
      if (m_Scenes.size) m_Scenes.Back()->Awake();
    }

    void FlushDeferred(void) {
      if (m_PendingPop) {
        PopNow();
        m_PendingPop = false;
      }
    }

    cbn::List<Scene *> m_Scenes;
    bool m_PendingPop {false};
  };

  /**
   * @brief ...
   */
  struct AssetManager final {
    AssetManager(const AssetManager &) = delete;
    AssetManager(AssetManager &&) = delete;
    AssetManager &operator=(const AssetManager &) = delete;
    AssetManager &operator=(AssetManager &&) = delete;

    static AssetManager &Get(void) {
      static AssetManager instance;
      return instance;
    }

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

    AssetManager(void)
      : m_AssetPack{cbn::SKAP::Open(GPH_ASSETPACK)}
    {
      if (!m_AssetPack) CARBON_UNREACHABLE;
      cbn::sprite_mgr::Init();
      cbn::audio::Init();
    }
  };

  /**
   * @brief ...
   */
  struct Game final {
    struct Spec {
      usz width, height;
      const char *title;
    };

    explicit Game(const Spec &s)
      : m_Canvas{cbn::DrawCanvas::New(s.width, s.height)}
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

    template <ValidEntity E, RenderLayer::T L, typename... Args>
    requires RenderLayer::Valid<L>
    E &NewEntity(Args &&... args) {
      return m_GlobalPool.Push<E, L>(args...);
    }

    template <ValidScene S>
    void InitScene(void) {
      m_SceneMgr.Switch<S>(*m_Canvas);
    }

    void Run(void) {
      cbn::win::ForFrame([this](const auto dt){
        m_SceneMgr.Update(dt);
        m_GlobalPool.Update(dt);
        m_SceneMgr.Render();
        m_GlobalPool.Render(*m_Canvas);
        m_Canvas->UpdateWindow();
      });
    }

  private:
    cbn::Scope<cbn::DrawCanvas> m_Canvas;
    SceneManager m_SceneMgr;
    EntityPool m_GlobalPool;
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
