// SPDX-License-Identifier: MIT
// Copyright (C) Wasym A. Alonso. All Rights Reserved.

#pragma once

#include <carbon.h>

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

    virtual void OnEnter(void) {}
    virtual void OnExit(void) {}
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
    explicit SceneManager(void) = default;

    SceneManager(const SceneManager &) = delete;
    SceneManager(SceneManager &&) = delete;
    SceneManager &operator=(const SceneManager &) = delete;
    SceneManager &operator=(SceneManager &&) = delete;

    ~SceneManager(void) {
      if (m_Current) delete m_Current;
      if (m_Next) delete m_Next;
    }

    void Update(const f64 dt) const {
      if (m_Current) m_Current->UpdateAll(dt);
    }

    void Render(void) const {
      if (m_Current) m_Current->RenderAll();
    }

    template <ValidScene S>
    void Switch(cbn::DrawCanvas &dc, const AssetManager &a_mgr) {
      if (m_Next) delete m_Next;
      m_Next = new S{dc, *this, a_mgr};
    }

    void Process(void) {
      if (!m_Next) return;
      if (m_Current) {
        m_Current->OnExit();
        delete m_Current;
      }
      m_Current = m_Next;
      m_Next = nullptr;
      m_Current->OnEnter();
    }

  private:
    Scene *m_Current {nullptr};
    Scene *m_Next {nullptr};
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
      : m_Canvas{typeof(m_Canvas)::make(s.width, s.height)},
        m_AssetMgr{s.ap_path}
    {
      m_Canvas.OpenWindow(s.title);
    }

    Game(const Game &) = delete;
    Game(Game &&) = delete;
    Game &operator=(const Game &) = delete;
    Game &operator=(Game &&) = delete;

    ~Game(void) {
      cbn::win::Close();
      m_Canvas.Free();
    }

    template <ValidScene S>
    void InitScene(void) {
      m_SceneMgr.Switch<S>(m_Canvas, m_AssetMgr);
    }

    void Run(void) {
      cbn::win::ForFrame([this](const auto dt){
        m_SceneMgr.Process();
        m_SceneMgr.Update(dt);
        m_SceneMgr.Render();
        m_Canvas.UpdateWindow();
      });
    }

  private:
    cbn::DrawCanvas m_Canvas;
    SceneManager m_SceneMgr;
    AssetManager m_AssetMgr;
  };
}

// TODO: change SceneManager to use a stack.
// TODO: implement GameContext.
