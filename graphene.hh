/*
  Game engine C++ header-only library. MIT. See license statements at the end of this file.
  graphene - v0.1-dev - 2026-04-20

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

/**
 * @brief Library printable name.
 */
#define GPH_LIBNAME "SPARKY Graphene"

#define GPH_VERSION_STR "0.1-dev"

#include <carbon.h>
#if CARBON_VERSION_MAJOR != 0 || CARBON_VERSION_MINOR != 18
#error Carbon v0.18 is needed
#endif

#ifndef GPH_ASSETPACK
#define GPH_ASSETPACK "Assets.skap"
#endif

namespace gph {
  // Forward declarations
  struct SceneManager;

  namespace mem {
    template <typename T, typename... Args>
    T *New(Args &&... args) {
      return new (cbn::mem::Alloc(sizeof(T))) T{cbn::meta::Forward<Args>(args)...};
    }

    template <typename T>
    void Delete(T *p) {
      p->~T();
      cbn::mem::Free(p);
    }
  }

  /**
   * @brief ...
   */
  struct Entity {
    Entity(void) = default;
    Entity(const Entity &) = delete;
    Entity(Entity &&) = delete;
    Entity &operator=(const Entity &) = delete;
    Entity &operator=(Entity &&) = delete;
    virtual ~Entity(void) = default;
    virtual void Update([[maybe_unused]] const f64 dt) {}
    virtual void Render([[maybe_unused]] cbn::DrawCanvas &dc) const {}
  };

  template <typename T>
  concept ValidEntity = cbn::meta::IsConvertible_v<T *, Entity *>;

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

  /**
   * @brief ...
   */
  struct EntityPool final {
    explicit EntityPool(cbn::DrawCanvas &dc)
      : r_Canvas{dc}
    {}

    ~EntityPool(void) {
      for (auto &l : m_Entities) {
        for (auto e : l) mem::Delete(e);
      }
    }

    usz Count(void) const {
      usz n = 0;
      for (const auto &l : m_Entities) n += l.size;
      return n;
    }

    template <ValidEntity E, RenderLayer::T L, typename... Args>
    requires RenderLayer::Valid<L>
    E &Push(Args &&... args) {
      auto e = mem::New<E>(cbn::meta::Forward<Args>(args)...);
      m_Entities[L].Push(e);
      return *e;
    }

    template <ValidEntity E>
    E *FindFirst(void) const {
      for (const auto &l : m_Entities) {
        for (auto e : l) {
          if (auto p = dynamic_cast<E *>(e)) return p;
        }
      }
      return nullptr;
    }

    template <ValidEntity E>
    cbn::List<E *> Find(void) const {
      cbn::List<E *> ps;
      for (const auto &l : m_Entities) {
        for (auto e : l) {
          if (auto p = dynamic_cast<E *>(e)) ps.Push(p);
        }
      }
      return ps;
    }

    void Update(const f64 dt) {
      for (const auto &l : m_Entities) {
        for (auto e : l) e->Update(dt);
      }
    }

    void Render(void) const {
      for (const auto &l : m_Entities) {
        for (auto e : l) e->Render(r_Canvas);
      }
    }

  private:
    cbn::Array<cbn::List<Entity *>, RenderLayer::Count> m_Entities;
    cbn::DrawCanvas &r_Canvas;
  };

  /**
   * @brief ...
   */
  struct Scene {
    explicit Scene(cbn::DrawCanvas &dc, SceneManager &s_mgr, const EntityPool &gp)
      : r_Canvas{dc}, r_SceneMgr{s_mgr}, r_GlobalPool{gp}
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

    usz EntityCount(void) const { return m_Pool.Count(); }

    void UpdateAll(const f64 dt) {
      Update(dt);
      m_Pool.Update(dt);
    }

    void RenderAll(void) const {
      Render();
      m_Pool.Render();
    }

  protected:
    cbn::DrawCanvas &r_Canvas;
    SceneManager &r_SceneMgr;

    template <ValidEntity E, RenderLayer::T L, typename... Args>
    requires RenderLayer::Valid<L>
    E &NewEntity(Args &&... args) {
      return m_Pool.Push<E, L>(cbn::meta::Forward<Args>(args)...);
    }

    template <ValidEntity E>
    E *FindFirstEntity(void) const { return m_Pool.FindFirst<E>(); }

    template <ValidEntity E>
    cbn::List<E *> FindEntities(void) const { return m_Pool.Find<E>(); }

    template <ValidEntity E>
    E *FindFirstGlobalEntity(void) const { return r_GlobalPool.FindFirst<E>(); }

    template <ValidEntity E>
    cbn::List<E *> FindGlobalEntities(void) const { return r_GlobalPool.Find<E>(); }

  private:
    EntityPool m_Pool {r_Canvas};
    const EntityPool &r_GlobalPool;
  };

  template <typename T>
  concept ValidScene = cbn::meta::IsConvertible_v<T *, Scene *>;

  /**
   * @brief ...
   */
  struct SceneManager final {
    explicit SceneManager(EntityPool &gp)
      : r_GlobalPool{gp}
    {}

    SceneManager(const SceneManager &) = delete;
    SceneManager(SceneManager &&) = delete;
    SceneManager &operator=(const SceneManager &) = delete;
    SceneManager &operator=(SceneManager &&) = delete;

    ~SceneManager(void) { Drain(); }

    usz Count(void) const { return m_Scenes.size; }

    usz CurrentSceneEntityCount(void) const {
      if (!m_Scenes.size) return 0;
      return m_Scenes.Back()->EntityCount();
    }

    template <ValidScene S>
    void Switch(cbn::DrawCanvas &dc) {
      m_PendingSwitch = [&]{
        Drain();
        Push<S>(dc);
      };
    }

    template <ValidScene S>
    void Push(cbn::DrawCanvas &dc) {
      auto s = mem::New<S>(dc, *this, r_GlobalPool);
      if (m_Scenes.size) m_Scenes.Back()->Snooze();
      m_Scenes.Push(s);
      s->Born();
    }

    void Pop(void) { m_PendingPop = true; }

    void Update(const f64 dt) {
      if (m_Scenes.size) m_Scenes.Back()->UpdateAll(dt);
      FlushDeferred();
    }

    void Render(void) const {
      if (m_Scenes.size) m_Scenes.Back()->RenderAll();
    }

  private:
    cbn::List<Scene *> m_Scenes;
    bool m_PendingPop {false};
    cbn::Func<void()> m_PendingSwitch;
    EntityPool &r_GlobalPool;

    void Drain(void) { while (m_Scenes.size) PopNow(); }

    void PopNow(void) {
      if (!m_Scenes.size) return;
      auto curr = m_Scenes.Back();
      curr->Die();
      mem::Delete(curr);
      m_Scenes.PopBack();
      if (m_Scenes.size) m_Scenes.Back()->Awake();
    }

    void FlushDeferred(void) {
      if (m_PendingSwitch) {
        m_PendingSwitch();
        m_PendingSwitch = nullptr;
      }
      if (m_PendingPop) {
        PopNow();
        m_PendingPop = false;
      }
    }
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

    u64 GetVersion(void) { return m_AssetPack->header.build_ver; }

  private:
    cbn::Opt<cbn::SKAP> m_AssetPack;

    AssetManager(void)
      : m_AssetPack{cbn::SKAP::Open(GPH_ASSETPACK)}
    {
      if (!m_AssetPack) CARBON_UNREACHABLE;
      cbn::audio::Init();
      cbn::mesh_mgr::Init();
      cbn::sprite_mgr::Init();
    }

    ~AssetManager(void) {
      cbn::sprite_mgr::Shutdown();
      cbn::mesh_mgr::Shutdown();
      cbn::audio::Shutdown();
      m_AssetPack->Free();
    }

    auto LoadAsset(auto load, const char *name) const {
      auto s = ((*m_AssetPack).*load)(name);
      if (!s) CARBON_UNREACHABLE;
      return *s;
    }

  public:
    cbn::Span<u8> LoadBinary(const char *name) const {
      return LoadAsset(&cbn::SKAP::Lookup<cbn::Span<u8>>, name);
    }

    cbn::sprite_mgr::UID LoadSprite(const char *name) const {
      return LoadAsset(&cbn::SKAP::LoadSprite, name);
    }

    cbn::mesh_mgr::UID LoadMesh(const char *name) const {
      return LoadAsset(&cbn::SKAP::LoadMesh, name);
    }
  };

  /**
   * @brief ...
   */
  struct DebugScreen final {
    explicit DebugScreen(cbn::DrawCanvas &dc, const SceneManager &s_mgr, const EntityPool &gp)
      : r_Canvas{dc}, r_SceneMgr{s_mgr}, r_GlobalPool{gp}
    {}

    DebugScreen(const DebugScreen &) = delete;
    DebugScreen(DebugScreen &&) = delete;
    DebugScreen &operator=(const DebugScreen &) = delete;
    DebugScreen &operator=(DebugScreen &&) = delete;

    void Update(const f64 dt) {
      m_FrameTime = dt;
      if (cbn::win::GetKeyDown(cbn::win::KeyCode::F3)) {
        m_Visible ^= true;
      }
    }

    void Render(void) const {
      if (!m_Visible) return;
      static constexpr auto txt_clr = 0x737373ff;
      static constexpr auto txt_sz = 2;
      static const auto txt_h = r_Canvas.TextHeight(txt_sz);
      const char *txt[] {
        "Engine (" GPH_LIBNAME ") " GPH_VERSION_STR,
        cbn::str::fmt("Core (" CARBON_LIBNAME ") %s", cbn::VersionStr()),
        cbn::str::fmt("AssetPack %llu", AssetManager::Get().GetVersion()),
        cbn::str::fmt("%u fps (%.4f ms)", cbn::win::GetFPS(), m_FrameTime),
        cbn::str::fmt("Canvas resolution: %zux%zu", r_Canvas.Width(), r_Canvas.Height()),
        cbn::str::fmt("Window resolution: %zux%zu", cbn::win::Width(), cbn::win::Height()),
        cbn::str::fmt("Scenes in stack: %zu", r_SceneMgr.Count()),
        cbn::str::fmt("Scene entities: %zu", r_SceneMgr.CurrentSceneEntityCount()),
        cbn::str::fmt("Global entities: %zu", r_GlobalPool.Count()),
        cbn::str::fmt("Memory usage: %.2f MiB", (f64)cbn::mem::Usage()/(1<<20))
      };
      for (usz i = 0; i < CARBON_ARRAY_LEN(txt); ++i) {
        r_Canvas.DrawText(txt[i], cbn::math::Vec2(10, 10 + i*txt_h), txt_sz, txt_clr);
      }
    }

  private:
    bool m_Visible {false};
    f64 m_FrameTime;
    cbn::DrawCanvas &r_Canvas;
    const SceneManager &r_SceneMgr;
    const EntityPool &r_GlobalPool;
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
      : m_Canvas{cbn::DrawCanvas::New(s.width, s.height)},
        m_SceneMgr{m_GlobalPool},
        m_DebugScr{*m_Canvas, m_SceneMgr, m_GlobalPool}
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
        m_DebugScr.Update(dt);
        m_SceneMgr.Render();
        m_GlobalPool.Render();
        m_DebugScr.Render();
        m_Canvas->UpdateWindow();
      });
    }

  private:
    cbn::Scope<cbn::DrawCanvas> m_Canvas;
    SceneManager m_SceneMgr;
    EntityPool m_GlobalPool {*m_Canvas};
    DebugScreen m_DebugScr;
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
