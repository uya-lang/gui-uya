# UyaGUI 性能优化指南

## 先看哪里

当前性能关键点主要集中在：

- `core/dirty_region.uya`
- `render/batch.uya`
- `render/gpu.uya`
- `render/zerocopy.uya`
- `res/cache.uya`

## 已有优化

### 脏区

- 包含关系去重
- 触碰区域自动合并
- 溢出时优先近似压缩，而不是直接全屏刷新
- `coverage_percent()` / `merge_ops` 可观测

### 批处理

- 同色填充矩形横向/纵向合并
- 水平线段拼接
- `gpu_execute_batch()` 按段下发批量命令

### 零拷贝

- 多传输区域队列
- 自动合并相邻区域
- 支持 `present_dirty()`

### 资源缓存

- 命中率、压力百分比、剩余预算
- LRU 风格淘汰

## 如何量化

推荐命令：

```bash
make bench
make bench-report
```

产物：

- [phase5_bench.txt](/home/winger/uya/gui-uya/build/phase5_bench.txt:1)
- [gui_uya_phase5_report.md](./gui_uya_phase5_report.md)

## 优化顺序建议

1. 先看是否真的需要更多像素填充
2. 再看是否能减小 dirty region 数量
3. 再看 batch 是否被充分合并
4. 再看缓存预算是否合适
5. 最后再决定是否需要真实 DMA / 硬件 GPU 后端
