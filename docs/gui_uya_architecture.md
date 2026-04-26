# UyaGUI 架构图

## 系统架构图

```text
App / Demo
  -> Widget Layer
    -> Core / Layout / Anim / Style
      -> Render
        -> Platform / Resource
          -> FrameBuffer / Input / Tick / FS
```

## 数据流图

```text
InputDev
  -> EventQueue
    -> EventDispatcher
      -> GuiObj tree
        -> invalidate()
          -> DirtyRegion
            -> DrawBatch / RenderCtx
              -> ZeroCopy / DisplayCtx
```

## 模块依赖图

```text
widget/*  -> core/*, render/*, style/*
layout/*  -> core/obj, core/rect, style/prop
anim/*    -> core/obj, style/prop
render/*  -> core/{rect,color,dirty_region}, platform/disp
res/*     -> render/img, std async/libc
platform/*-> core/event, core/point, core/rect
```

## 内存布局图

```text
Static / Stack
  - GuiObj / Widget / Panel / Page / Chart
  - DrawBatch
  - DirtyRegion
  - ThemeManager
  - Example framebuffers

Managed Pools / Cache
  - MemPool / PoolManager
  - ObjPool<T>
  - ImageCache entries

Display Buffers
  - front framebuffer
  - back framebuffer
  - optional canvas buffers
```

## 相关阅读

- [gui_uya_design.md](./gui_uya_design.md)
- [gui_uya_phase5_report.md](./gui_uya_phase5_report.md)
- [gui_uya_performance_guide.md](./gui_uya_performance_guide.md)
