# UyaGUI Phase 5 报告

> 日期: 2026-04-26  
> 范围: 当前仓库内可直接实现与验证的 `Phase 5` 子项  
> 备注: 目标板/显示屏/真实 DMA 或硬件 GPU 驱动实测不在本次本地交付范围内

## 本次完成

- `core/dirty_region.uya`
  - 改进脏区收敛策略，支持包含关系去重、全量合并、溢出时近似压缩而不是直接退化全屏刷新
  - 增加 `coverage_percent()` 与 `merge_ops` 统计
- `render/batch.uya`
  - 批处理优化从“同色矩形横向合并”扩展到“横向/纵向矩形合并 + 水平线段拼接”
- `render/gpu.uya`
  - 新增 `gpu_execute_batch()`，按命令段批量下发到 GPU 接口，线段保留软件回退
- `render/zerocopy.uya`
  - 从单一区域传输升级为多脏区队列、自动合并、按 dirty region 提交与统计
- `res/cache.uya`
  - 增加缓存命中率、压力百分比、剩余预算和统计重置接口
- 测试与脚手架
  - 新增 `tests/test_phase5_runtime.uya`
  - `make bench-report` 生成 `build/phase5_bench.txt`
  - CI 增加 benchmark 执行与 nightly/schedule 触发

## 验证结果

- `make test`
  - 84 个 GUI/runtime 测试通过
  - 7 个 render 测试通过
- `make build`
  - `build/phase4_smoke` 构建通过
- `make bench`
  - O0 基准已通过
- `make bench-report`
  - O3 报告已生成到 `build/phase5_bench.txt`

## 最新基准

以下数据来自 `make bench-report`（`-O3`，5000 iterations）：

| 项目 | 数值 |
|------|------|
| `DirtyRegion` 结构体大小 | `208 bytes` |
| `DrawBatch` 结构体大小 | `4616 bytes` |
| `RenderCtx` 结构体大小 | `408 bytes` |
| `ZeroCopyCtx` 结构体大小 | `160 bytes` |
| `ImageCache` 结构体大小 | `928 bytes` |
| `FB 480x320 RGB565` | `307200 bytes` |
| `FB 320x240 RGB565` | `153600 bytes` |
| `obj_pool` | `1 ms` |
| `allocator` | `0 ms` |
| `layout+dirty` | `1 ms` |
| `dispatcher` | `5 ms` |
| `render prim` | `525 ms` |
| `image+batch` | `125 ms` |
| `gpu batch` | `51 ms` |
| `cache churn` | `3 ms` |
| `startup init` | `1019 us` |
| `32 anims` | `13 ms` |

## 尚未完成

- 与 LVGL 的对比基准
- 真实 DMA 后端
- 真实硬件 GPU 后端
- STM32 / ESP32 / RP2040 / RISC-V 目标板实测
- 多分辨率显示屏实机适配验证
