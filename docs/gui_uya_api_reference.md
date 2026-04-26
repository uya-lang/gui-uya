# UyaGUI API Reference

> 该文件由 `tools/gen_gui_api_docs.sh` 从 `gui/**/*.uya` 自动生成。
> 内容聚焦公开符号索引，详细设计说明请结合 `docs/gui_uya_*.md` 系列文档阅读。

## `gui/anim/easing.uya`

- L50: `export enum EasingType {`
- L78: `export fn easing_variant_count() i32 {`
- L82: `export fn apply_easing(t: f32, easing: EasingType) f32 {`

## `gui/anim/timeline.uya`

- L6: `export const MAX_ANIMATIONS: i32 = 32;`
- L10: `export struct AnimManager {`
- L91: `export fn anim_manager_new() AnimManager {`

## `gui/anim/tween.uya`

- L87: `export enum AnimProp {`
- L110: `export interface ITweenCompleteCallback {`
- L114: `export interface ITweenUpdateCallback {`
- L118: `export struct Tween {`
- L436: `export fn anim_value_i32(value: i32) AnimValue {`
- L440: `export fn anim_value_f32(value: f32) AnimValue {`
- L444: `export fn anim_value_color(value: Color) AnimValue {`

## `gui/benchmarks/core_bench.uya`

- L306: `export fn run_core_bench() i32 {`

## `gui/bench_suite.uya`

- L5: `export fn main() i32 {`

## `gui/core/bitmap.uya`

- L6: `export const BITMAP_ALLOCATOR_MAX_BITS: i32 = 256;`
- L31: `export struct BitmapAllocator {`
- L124: `export fn bitmap_allocator_new(capacity: i32) BitmapAllocator {`

## `gui/core/color.uya`

- L21: `export struct Color {`
- L28: `export const BLACK: Color = Color{ r: 0, g: 0, b: 0, a: 255 };`
- L29: `export const WHITE: Color = Color{ r: 255, g: 255, b: 255, a: 255 };`
- L30: `export const RED: Color = Color{ r: 255, g: 0, b: 0, a: 255 };`
- L31: `export const GREEN: Color = Color{ r: 0, g: 255, b: 0, a: 255 };`
- L32: `export const BLUE: Color = Color{ r: 0, g: 0, b: 255, a: 255 };`
- L33: `export const TRANSPARENT: Color = Color{ r: 0, g: 0, b: 0, a: 0 };`
- L35: `export mc COLOR(hex: expr) expr {`

## `gui/core/dirty_region.uya`

- L5: `export const MAX_DIRTY_PRECISE: i32 = 16;`
- L6: `export const MAX_DIRTY_MERGED: i32 = 8;`
- L34: `export struct DirtyRegionView {`
- L39: `export struct DirtyRegion {`
- L237: `export fn dirty_region_new() DirtyRegion {`

## `gui/core/event_dispatch.uya`

- L35: `export struct EventDispatcher {`
- L232: `export fn event_dispatcher_new() EventDispatcher {`

## `gui/core/event.uya`

- L4: `export const EVENT_QUEUE_SIZE: i32 = 64;`
- L19: `export enum EventType {`
- L50: `export enum EventPhase {`
- L56: `export enum InputDev {`
- L64: `export struct EventPointValue {`
- L77: `export struct Event {`
- L114: `export struct EventOption {`
- L119: `export struct EventQueue {`
- L162: `export struct GestureDetector {`
- L287: `export struct GestureConfig {`
- L296: `export fn gesture_config_default() GestureConfig {`
- L307: `export fn gesture_detector_with_config(config: GestureConfig) GestureDetector {`
- L323: `export fn event_with_phase(evt: Event, phase: EventPhase) Event {`
- L329: `export fn event_point_value(kind: EventType, dev: InputDev, target: i32, timestamp: u32, point: Point, value: i32) Event {`
- L345: `export fn event_focus(kind: EventType, target: i32, timestamp: u32) Event {`
- L358: `export fn event_none() Event {`
- L371: `export fn event_option_some(evt: Event) EventOption {`
- L378: `export fn event_option_none() EventOption {`
- L385: `export fn event_point(kind: EventType, dev: InputDev, target: i32, timestamp: u32, point: Point) Event {`
- L398: `export fn event_key(kind: EventType, dev: InputDev, target: i32, timestamp: u32, key_code: u16) Event {`
- L411: `export fn event_value(kind: EventType, dev: InputDev, target: i32, timestamp: u32, value: i32) Event {`
- L424: `export fn event_queue_new() EventQueue {`
- L434: `export fn gesture_detector_new() GestureDetector {`

## `gui/core/obj_pool.uya`

- L1: `export const OBJ_POOL_CAPACITY: i32 = 128;`
- L5: `export enum ObjSlotState {`
- L11: `export struct ObjPoolIndices {`
- L16: `export struct ObjPool<T: IGuiObj> {`
- L98: `export fn obj_pool_new<T: IGuiObj>() ObjPool<T> {`

## `gui/core/obj_tree.uya`

- L7: `export const OBJ_TREE_MAX_PATH: i32 = 64;`
- L9: `export struct BubblePath {`
- L22: `export struct ObjTree {`
- L201: `export fn obj_tree_new() ObjTree {`

## `gui/core/obj.uya`

- L9: `export const GUI_OBJ_REGISTRY_CAPACITY: i32 = 256;`
- L11: `export interface IGuiObj {`
- L24: `export interface IContainer {`
- L31: `export interface IGuiEventCallback {`
- L35: `export interface IGuiRenderCallback {`
- L39: `export interface IGuiLayoutCallback {`
- L43: `export interface IGuiInputCallback {`
- L47: `export struct EventCallbackSlot {`
- L52: `export struct ObjFlags {`
- L63: `export struct GuiObj : IGuiObj, IContainer {`
- L102: `export fn obj_flags_default() ObjFlags {`
- L115: `export fn event_callback_slot_default() EventCallbackSlot {`
- L443: `export fn gui_obj_default() GuiObj {`
- L475: `export fn gui_obj_register(idx: i32, obj: &GuiObj) bool {`
- L488: `export fn gui_obj_get(idx: i32) &GuiObj {`
- L499: `export fn gui_obj_unregister(idx: i32) void {`
- L510: `export fn gui_obj_reset_registry() void {`

## `gui/core/point.uya`

- L20: `export struct Point {`
- L25: `export const POINT_ZERO: Point = Point{ x: 0, y: 0 };`
- L26: `export const POINT_ONE: Point = Point{ x: 1, y: 1 };`

## `gui/core/rect.uya`

- L21: `export struct Rect {`
- L28: `export const RECT_ZERO: Rect = Rect{ x: 0, y: 0, w: 0, h: 0 };`
- L148: `export fn rect_union(a: Rect, b: Rect) Rect {`

## `gui/core/size.uya`

- L1: `export struct Size {`
- L6: `export const SIZE_ZERO: Size = Size{ w: 0, h: 0 };`

## `gui/examples/custom/gauge.uya`

- L36: `export fn run_custom_gauge_example() i32 {`

## `gui/examples/custom/keyboard.uya`

- L9: `export fn run_custom_keyboard_example() i32 {`

## `gui/examples/demo_clock.uya`

- L12: `export fn run_demo_clock() i32 {`

## `gui/examples/demo_dashboard.uya`

- L19: `export fn run_demo_dashboard() i32 {`

## `gui/examples/demo_game.uya`

- L13: `export fn run_demo_game() i32 {`

## `gui/examples/demo_music.uya`

- L14: `export fn run_demo_music() i32 {`

## `gui/examples/demo_perf.uya`

- L13: `export fn run_demo_perf() i32 {`

## `gui/examples/demo_settings.uya`

- L13: `export fn run_demo_settings() i32 {`

## `gui/examples/phase0_smoke.uya`

- L17: `export fn run_phase0_smoke() i32 {`

## `gui/examples/phase1_smoke.uya`

- L20: `export fn run_phase1_smoke() i32 {`

## `gui/examples/phase2_smoke.uya`

- L26: `export fn run_phase2_smoke() i32 {`

## `gui/examples/phase3_smoke.uya`

- L33: `export fn run_phase3_smoke() i32 {`

## `gui/examples/phase4_smoke.uya`

- L45: `export fn run_phase4_smoke() i32 {`

## `gui/examples/phase6_smoke.uya`

- L10: `export fn run_phase6_smoke() i32 {`

## `gui/layout/abs.uya`

- L4: `export enum OverflowMode {`
- L60: `export struct AbsLayout {`
- L76: `export fn abs_layout_default() AbsLayout {`

## `gui/layout/auto.uya`

- L10: `export enum LayoutMode {`
- L17: `export struct AutoLayout {`
- L43: `export fn auto_layout_new() AutoLayout {`

## `gui/layout/flex.uya`

- L4: `export const MAX_FLEX_CHILDREN: i32 = 64;`
- L5: `export const MAX_FLEX_LINES: i32 = 16;`
- L7: `export enum FlexDir {`
- L14: `export enum Justify {`
- L23: `export enum Align {`
- L31: `export struct FlexConfig {`
- L160: `export fn flex_config_row() FlexConfig {`
- L174: `export fn flex_config_column() FlexConfig {`
- L180: `export struct FlexLayout {`
- L456: `export fn flex_layout_row() FlexLayout {`
- L460: `export fn flex_layout_column() FlexLayout {`

## `gui/layout/grid.uya`

- L4: `export const MAX_GRID_COLS: i32 = 8;`
- L5: `export const MAX_GRID_ROWS: i32 = 8;`
- L7: `export enum GridAutoFlow {`
- L12: `export struct GridConfig {`
- L46: `export struct GridLayout {`
- L109: `export fn grid_layout_default() GridLayout {`

## `gui/phase0_smoke.uya`

- L5: `export fn main() i32 {`

## `gui/phase1_smoke.uya`

- L5: `export fn main() i32 {`

## `gui/phase2_smoke.uya`

- L5: `export fn main() i32 {`

## `gui/phase3_smoke.uya`

- L5: `export fn main() i32 {`

## `gui/phase4_smoke.uya`

- L5: `export fn main() i32 {`

## `gui/phase6_smoke.uya`

- L5: `export fn main() i32 {`

## `gui/platform/disp.uya`

- L73: `export enum PixelFormat {`
- L84: `export struct FrameBuffer {`
- L91: `export struct DisplayCtx {`
- L116: `export fn bytes_per_pixel(format: PixelFormat) u8 {`
- L132: `export fn framebuffer_required_bytes(w: u16, h: u16, format: PixelFormat) usize {`
- L142: `export fn framebuffer_rect(fb: &FrameBuffer) Rect {`
- L146: `export fn framebuffer_new(pixels: &byte, w: u16, h: u16, stride: u16, format: PixelFormat) FrameBuffer {`
- L155: `export fn empty_framebuffer() FrameBuffer {`
- L164: `export fn display_ctx_new(front: FrameBuffer, back: FrameBuffer) DisplayCtx {`
- L172: `export fn framebuffer_inside(fb: &FrameBuffer, x: i32, y: i32) bool {`
- L176: `export fn framebuffer_get_pixel(fb: &FrameBuffer, x: i32, y: i32) Color {`
- L234: `export fn framebuffer_set_pixel(fb: &FrameBuffer, x: i32, y: i32, color: &Color) void {`
- L303: `export fn framebuffer_clear(fb: &FrameBuffer, color: Color) void {`
- L324: `export enum DisplayDriverKind {`
- L333: `export struct DisplayDriverProfile {`
- L343: `export fn display_driver_profile(kind: DisplayDriverKind) DisplayDriverProfile {`
- L410: `export fn display_driver_required_bytes(kind: DisplayDriverKind) usize {`
- L415: `export fn framebuffer_new_for_driver(pixels: &byte, kind: DisplayDriverKind) FrameBuffer {`

## `gui/platform/fb/disp_fb.uya`

- L21: `export struct FbDisplay {`
- L123: `export fn fb_display_new() FbDisplay {`
- L133: `export fn fb_display_last_error() &const byte {`

## `gui/platform/fb/indev_fb_common.uya`

- L7: `export const FB_EVT_NONE: u8 = 0u8;`
- L8: `export const FB_EVT_KEY_DOWN: u8 = 1u8;`
- L9: `export const FB_EVT_HOVER_MOVE: u8 = 2u8;`
- L10: `export const FB_EVT_TOUCH_DOWN: u8 = 3u8;`
- L11: `export const FB_EVT_TOUCH_UP: u8 = 4u8;`
- L12: `export const FB_EVT_TOUCH_MOVE: u8 = 5u8;`
- L13: `export const FB_EVT_WHEEL: u8 = 6u8;`
- L15: `export struct FbHostEvent {`
- L24: `export fn fb_host_event_none() FbHostEvent {`
- L35: `export fn fb_feed_host_event(`
- L86: `export fn fb_hover_point_default() Point {`

## `gui/platform/fb/indev_fb.uya`

- L22: `export struct FbInputSystem {`
- L96: `export fn fb_input_system_new() FbInputSystem {`
- L111: `export fn fb_input_last_error() &const byte {`

## `gui/platform/indev.uya`

- L18: `export const TOUCH_FILTER_WINDOW: i32 = 5;`
- L19: `export const MAX_INPUT_DEVICES: i32 = 8;`
- L68: `export interface IInputDev {`
- L73: `export struct TouchCalibSample {`
- L78: `export struct TouchCalibration {`
- L109: `export struct TouchMedianFilter {`
- L150: `export struct TouchDriver : IInputDev {`
- L201: `export struct MouseDriver : IInputDev {`
- L222: `export struct KeyDriver : IInputDev {`
- L243: `export struct EncoderDriver : IInputDev {`
- L260: `export struct InputDeviceSlot {`
- L266: `export struct InputManager {`
- L339: `export fn touch_calibration_from_samples(samples: &TouchCalibSample, count: i32) TouchCalibration {`
- L387: `export fn touch_driver_new() TouchDriver {`
- L402: `export fn mouse_driver_new() MouseDriver {`
- L406: `export fn key_driver_new() KeyDriver {`
- L410: `export fn encoder_driver_new() EncoderDriver {`
- L414: `export fn input_manager_new() InputManager {`
- L422: `export fn touch_calibration_identity() TouchCalibration {`
- L426: `export fn touch_filter_default() TouchMedianFilter {`

## `gui/platform/sdl2/disp_sdl.uya`

- L21: `export struct SdlDisplay {`
- L127: `export fn sdl_display_new() SdlDisplay {`
- L139: `export fn sdl_display_last_error() &const byte {`

## `gui/platform/sdl2/indev_sdl.uya`

- L26: `export struct SdlHostEvent {`
- L35: `export struct SdlInputSystem {`
- L141: `export fn sdl_input_system_new() SdlInputSystem {`

## `gui/platform/tick.uya`

- L12: `export fn get_tick_ms() u32 {`
- L18: `export fn get_tick_us() u64 {`
- L23: `export fn sleep_ms(ms: u32) void {`

## `gui/render/batch.uya`

- L11: `export const DRAW_BATCH_CAPACITY: i32 = 64;`
- L13: `export enum DrawCmdKind {`
- L20: `export struct DrawCmd {`
- L106: `export struct DrawBatch {`
- L215: `export fn draw_batch_new() DrawBatch {`

## `gui/render/ctx.uya`

- L19: `export const MAX_CLIP_STACK: i32 = 16;`
- L39: `export enum RenderMode {`
- L45: `export struct RenderStats {`
- L56: `export struct RenderCtx {`
- L327: `export fn empty_framebuffer_color() Color {`
- L331: `export fn render_ctx_new(fb: FrameBuffer) RenderCtx {`
- L353: `export fn empty_render_ctx() RenderCtx {`
- L357: `export fn default_ctx() RenderCtx {`
- L361: `export fn ctx_pixel(ctx: &RenderCtx, x: i16, y: i16) Color {`

## `gui/render/font.uya`

- L17: `export const BITMAP_FONT_GLYPH_CAPACITY: i32 = 128;`
- L18: `export const BITMAP_FONT_UNICODE_CAPACITY: i32 = 64;`
- L19: `export const BITMAP_FONT_KERNING_CAPACITY: i32 = 64;`
- L20: `export const TTF_POINT_CAPACITY: i32 = 2048;`
- L21: `export const TTF_CONTOUR_CAPACITY: i32 = 256;`
- L22: `export const TTF_EDGE_CAPACITY: i32 = 16384;`
- L23: `export const TTF_COMPOSITE_DEPTH_MAX: i32 = 8;`
- L24: `export const TTF_CACHE_ENTRY_CAPACITY: i32 = 128;`
- L25: `export const TTF_CACHE_SLOT_DIM: i32 = 64;`
- L26: `export const TTF_CACHE_SLOT_BYTES: i32 = TTF_CACHE_SLOT_DIM * TTF_CACHE_SLOT_DIM;`
- L27: `export const TTF_CACHE_STORAGE_BYTES: i32 = TTF_CACHE_ENTRY_CAPACITY * TTF_CACHE_SLOT_BYTES;`
- L29: `export enum TextAlign {`
- L35: `export enum FontBitmapFormat {`
- L41: `export enum FontRenderKind {`
- L53: `export struct Glyph {`
- L63: `export struct GlyphLookupEntry {`
- L68: `export struct GlyphLookupTable {`
- L75: `export struct KerningPair {`
- L81: `export struct Font {`
- L102: `export struct BitmapFontAsset {`
- L148: `export struct TtfFontAsset {`
- L1550: `export fn ttf_cache_reset() void {`
- L1572: `export fn ttf_cache_hit_count() u32 {`
- L1576: `export fn ttf_cache_miss_count() u32 {`
- L1649: `export fn ttf_font_set_hinting(asset: &TtfFontAsset, enabled: bool) void {`
- L1736: `export fn ttf_font_asset_new() TtfFontAsset {`
- L2029: `export fn ttf_font_load_memory(asset: &TtfFontAsset, name: &const byte, data: &const byte, len: usize, pixel_height: u16) bool {`
- L2225: `export fn bitmap_font_asset_new() BitmapFontAsset {`
- L2245: `export fn bitmap_font_load_bmfont(asset: &BitmapFontAsset, desc: &const byte, len: usize, bitmap_data: &const byte, bitmap_stride: u16) bool {`
- L2350: `export fn font_system_default() Font {`
- L2374: `export fn font_system_compact() Font {`
- L2398: `export fn font_system_vector() Font {`
- L2422: `export fn font_default() Font {`
- L2426: `export fn glyph_for_char(font: &Font, code: u8) Glyph {`
- L2430: `export fn text_width(text: &const byte, len: usize, font: &Font) i16 {`
- L2434: `export fn draw_text(ctx: &RenderCtx, x: i16, y: i16, text: &const byte, len: usize, font: &Font, color: Color) void {`
- L2460: `export fn draw_text_aligned(ctx: &RenderCtx, rect: Rect, text: &const byte, len: usize, font: &Font, color: Color, align: TextAlign) void {`

## `gui/render/gpu.uya`

- L10: `export interface IGpuCtx {`
- L17: `export struct SoftwareGpuCtx : IGpuCtx {`
- L57: `export fn software_gpu_new(ctx: &RenderCtx) SoftwareGpuCtx {`
- L64: `export fn gpu_execute_batch(gpu: &IGpuCtx, ctx: &RenderCtx, batch: &DrawBatch) void {`

## `gui/render/img.uya`

- L26: `export struct ImageData {`
- L34: `export fn image_data_new(pixels: &byte, w: u16, h: u16, stride: u16, format: PixelFormat) ImageData {`
- L44: `export fn empty_image() ImageData {`
- L54: `export fn image_size_bytes(img: &ImageData) u32 {`
- L64: `export fn image_retain(img: &ImageData) void {`
- L68: `export fn image_release(img: &ImageData) void {`
- L74: `export fn draw_image(ctx: &RenderCtx, x: i16, y: i16, img: &ImageData) void {`
- L90: `export fn draw_image_scaled(ctx: &RenderCtx, rect: Rect, img: &ImageData) void {`
- L108: `export fn draw_image_clipped(ctx: &RenderCtx, dst_rect: Rect, img: &ImageData, src_rect: Rect) void {`
- L126: `export fn draw_image_rotated(ctx: &RenderCtx, x: i16, y: i16, img: &ImageData, angle_deg: i16) void {`

## `gui/render/zerocopy.uya`

- L8: `export const ZERO_COPY_TRANSFER_CAPACITY: i32 = 8;`
- L22: `export struct ZeroCopyTransfer {`
- L28: `export struct ZeroCopyCtx {`
- L156: `export fn zerocopy_ctx_new(display: &DisplayCtx) ZeroCopyCtx {`

## `gui/res/buf.uya`

- L3: `export struct Slice<T> {`
- L8: `export struct ByteSlice {`
- L13: `export struct Buffer {`
- L77: `export fn buffer_new(allocator: &MallocAllocator) Buffer {`
- L86: `export fn slice_from_ptr<T>(ptr: &T, len: usize) Slice<T> {`

## `gui/res/cache.uya`

- L8: `export const IMAGE_CACHE_CAPACITY: i32 = 16;`
- L10: `export enum CacheEntryState {`
- L17: `export struct CacheEntry {`
- L27: `export struct ImageCache {`
- L275: `export @async_fn fn image_cache_put_async(cache: &ImageCache, key: u32, image: ImageData) Future<!bool> {`
- L282: `export fn image_cache_new(capacity: u8, budget_bytes: u32) ImageCache {`

## `gui/res/fs.uya`

- L8: `export const ROM_FS_CAPACITY: i32 = 16;`
- L9: `export const FS_PATH_CAPACITY: i32 = 256;`
- L93: `export interface IFileSystem {`
- L98: `export @async_fn fn fs_read_async<T: IFileSystem>(fs: &T, path: &const byte, out: &byte, capacity: usize) Future<!usize> {`
- L102: `export struct RomFsEntry {`
- L109: `export struct RomFileSystem : IFileSystem {`
- L159: `export struct FatFileSystem : IFileSystem {`
- L196: `export fn rom_fs_new() RomFileSystem {`
- L203: `export fn fat_fs_new(base_dir: &const byte) FatFileSystem {`
- L207: `export fn fs_path_len(path: &const byte) usize {`

## `gui/res/pool.uya`

- L4: `export const MEM_POOL_BYTES: i32 = 4096;`
- L31: `export struct MemPool {`
- L79: `export struct PoolManager {`
- L110: `export struct TypedPool<T> {`
- L134: `export fn mem_pool_new(block_size: u16, block_count: u16) MemPool {`
- L156: `export fn pool_manager_new() PoolManager {`
- L164: `export fn typed_pool<T>(pool: &MemPool) TypedPool<T> {`

## `gui/sim/app.uya`

- L91: `export struct SimApp {`
- L751: `export fn sim_app_new() SimApp {`

## `gui/sim/common.uya`

- L1: `export const SIM_TEXT_CAPACITY: i32 = 160;`
- L2: `export const SIM_PATH_CAPACITY: i32 = 256;`
- L4: `export const SIM_KEY_ESCAPE: u16 = 27;`
- L5: `export const SIM_KEY_ENTER: u16 = 13;`
- L6: `export const SIM_KEY_SPACE: u16 = 32;`
- L7: `export const SIM_KEY_LEFT: u16 = 1000;`
- L8: `export const SIM_KEY_RIGHT: u16 = 1001;`
- L9: `export const SIM_KEY_UP: u16 = 1002;`
- L10: `export const SIM_KEY_DOWN: u16 = 1003;`
- L11: `export const SIM_KEY_F11: u16 = 1011;`
- L13: `export fn sim_cstr_empty(text: &const byte) bool {`
- L17: `export fn sim_cstr_eq(a: &const byte, b: &const byte) bool {`
- L33: `export fn sim_copy_cstr(dst: &byte, capacity: usize, src: &const byte) bool {`

## `gui/sim/config.uya`

- L15: `export enum SimDemoKind {`
- L20: `export enum SimBackendKind {`
- L25: `export struct SimConfig {`
- L46: `export fn sim_demo_name(kind: SimDemoKind) &const byte {`
- L53: `export fn sim_backend_name(kind: SimBackendKind) &const byte {`
- L60: `export fn sim_config_default() SimConfig {`
- L83: `export fn sim_config_from_runtime() SimConfig {`

## `gui/sim/main.uya`

- L3: `export fn sim_entry() i32 {`

## `gui/sim_main.uya`

- L5: `export fn main() i32 {`

## `gui/sim/profiler.uya`

- L3: `export struct ProfileStat {`
- L30: `export struct ProfilerSnapshot {`
- L39: `export struct SimProfiler {`
- L88: `export fn profile_stat_new() ProfileStat {`
- L98: `export fn sim_profiler_new(report_every_frames: u32) SimProfiler {`

## `gui/sim/recorder.uya`

- L10: `export const SIM_RECORDER_CAPACITY: i32 = 1024;`
- L11: `export const SIM_RECORDER_MAGIC: u32 = 0x52435955u32;`
- L12: `export const SIM_RECORDER_VERSION: u16 = 1u16;`
- L17: `export enum RecorderEventKind {`
- L27: `export struct RecordedEvent {`
- L43: `export struct SimRecorder {`
- L181: `export fn sim_recorder_new() SimRecorder {`

## `gui/sim/runner.uya`

- L236: `export fn run_simulator() i32 {`

## `gui/sim/screenshot.uya`

- L17: `export const SIM_SCREENSHOT_MAGIC: u32 = 0x46425559u32;`
- L18: `export const SIM_SCREENSHOT_VERSION: u16 = 1u16;`
- L51: `export fn screenshot_payload_bytes(fb: &FrameBuffer) usize {`
- L229: `export fn screenshot_write_png(path: &const byte, fb: &FrameBuffer) !usize {`
- L315: `export fn screenshot_write_bmp(path: &const byte, fb: &FrameBuffer) !usize {`
- L380: `export fn screenshot_write_raw(path: &const byte, fb: &FrameBuffer) !usize {`
- L412: `export fn screenshot_write(path: &const byte, fb: &FrameBuffer) !usize {`

## `gui/style/prop.uya`

- L1: `export const MAX_STYLE_EXT_PROPS: i32 = 24;`
- L3: `export enum StyleProp {`

## `gui/style/style.uya`

- L143: `export enum StyleValueKind {`
- L152: `export struct StyleEntry {`
- L162: `export struct Style {`
- L674: `export struct StyleBuilder {`
- L785: `export fn style_builder_new() StyleBuilder {`
- L789: `export const DEFAULT_STYLE: Style = Style{`
- L818: `export fn style_empty() Style {`
- L822: `export fn style_default() Style {`
- L826: `export fn style_make(bg: Color, fg: Color, border_width: u8, border_color: Color, radius: u8, opacity: u8) Style {`
- L836: `export mc style(bg: expr, fg: expr, border_width: expr, border_color: expr, radius: expr, opacity: expr) expr {`

## `gui/style/theme.uya`

- L11: `export const MAX_THEMES: i32 = 8;`
- L25: `export enum ThemeVariant {`
- L31: `export struct Theme {`
- L47: `export struct ThemeManager {`
- L171: `export fn theme_manager_new() ThemeManager {`
- L180: `export fn material_light_theme() Theme {`
- L198: `export fn material_dark_theme() Theme {`
- L216: `export fn compact_theme() Theme {`

## `gui/tests/test_anim.uya`

- L26: `export const TEST_ANIM_MODULE: i32 = 1;`

## `gui/tests/test_bitmap.uya`

- L7: `export const TEST_BITMAP_MODULE: i32 = 1;`

## `gui/tests/test_color.uya`

- L13: `export const TEST_COLOR_MODULE: i32 = 1;`

## `gui/tests/test_core_types.uya`

- L12: `export const TEST_CORE_TYPES_MODULE: i32 = 1;`

## `gui/tests/test_dirty_region.uya`

- L8: `export const TEST_DIRTY_REGION_MODULE: i32 = 1;`

## `gui/tests/test_event_dispatch.uya`

- L20: `export const TEST_EVENT_DISPATCH_MODULE: i32 = 1;`

## `gui/tests/test_event.uya`

- L17: `export const TEST_EVENT_MODULE: i32 = 1;`

## `gui/tests/test_input_dev.uya`

- L34: `export const TEST_INPUT_DEV_MODULE: i32 = 1;`

## `gui/tests/test_layout.uya`

- L23: `export const TEST_LAYOUT_MODULE: i32 = 1;`

## `gui/tests/test_obj_tree.uya`

- L14: `export const TEST_OBJ_TREE_MODULE: i32 = 1;`

## `gui/tests/test_phase4_io.uya`

- L23: `export const TEST_PHASE4_IO_MODULE: i32 = 1;`

## `gui/tests/test_phase5_runtime.uya`

- L45: `export const TEST_PHASE5_RUNTIME_MODULE: i32 = 1;`

## `gui/tests/test_phase6_examples.uya`

- L10: `export const TEST_PHASE6_EXAMPLES_MODULE: i32 = 1;`

## `gui/tests/test_pool.uya`

- L20: `export const TEST_POOL_MODULE: i32 = 1;`

## `gui/tests/test_rect.uya`

- L8: `export const TEST_RECT_MODULE: i32 = 1;`

## `gui/tests/test_render_assets.uya`

- L42: `export const TEST_RENDER_ASSETS_MODULE: i32 = 1;`

## `gui/tests/test_render_ctx.uya`

- L17: `export const TEST_RENDER_CTX_MODULE: i32 = 1;`

## `gui/tests/test_render_pipeline.uya`

- L23: `export const TEST_RENDER_PIPELINE_MODULE: i32 = 1;`

## `gui/tests/test_sim_app.uya`

- L16: `export const TEST_SIM_APP_MODULE: i32 = 1;`

## `gui/tests/test_sim_tools.uya`

- L22: `export const TEST_SIM_TOOLS_MODULE: i32 = 1;`

## `gui/tests/test_style.uya`

- L24: `export const TEST_STYLE_MODULE: i32 = 1;`

## `gui/tests/test_utils.uya`

- L8: `export fn _assert_eq_impl<T>(actual: &T, expected: &T, actual_src: &const byte, expected_src: &const byte) !void {`
- L15: `export mc assert_eq(actual: expr, expected: expr) expr {`
- L19: `export fn _assert_near_impl(actual: f64, expected: f64, epsilon: f64, actual_src: &const byte, expected_src: &const byte) !void {`
- L26: `export mc assert_near(actual: expr, expected: expr, epsilon: expr) expr {`
- L30: `export fn _test_suite_impl(name: &const byte) void {`
- L34: `export mc test_suite(name: expr) expr {`

## `gui/tests/test_widgets.uya`

- L39: `export const TEST_WIDGET_MODULE: i32 = 1;`

## `gui/widget/base.uya`

- L25: `export const WIDGET_TYPE_WIDGET: u32 = 0x1000u32;`
- L26: `export const WIDGET_TYPE_BUTTON: u32 = 0x1001u32;`
- L27: `export const WIDGET_TYPE_LABEL: u32 = 0x1002u32;`
- L28: `export const WIDGET_TYPE_IMAGE: u32 = 0x1003u32;`
- L29: `export const WIDGET_TYPE_SLIDER: u32 = 0x1004u32;`
- L30: `export const WIDGET_TYPE_SWITCH: u32 = 0x1005u32;`
- L31: `export const WIDGET_TYPE_PAGE: u32 = 0x1006u32;`
- L32: `export const WIDGET_TYPE_PANEL: u32 = 0x1007u32;`
- L33: `export const WIDGET_TYPE_LIST: u32 = 0x1008u32;`
- L34: `export const WIDGET_TYPE_GRID_VIEW: u32 = 0x1009u32;`
- L35: `export const WIDGET_TYPE_CHART: u32 = 0x100Au32;`
- L36: `export const WIDGET_TYPE_CANVAS: u32 = 0x100Bu32;`
- L115: `export enum WidgetState {`
- L124: `export enum WidgetOrientation {`
- L129: `export interface IStyled {`
- L135: `export interface IWidgetEventCallback {`
- L139: `export interface IWidgetValueCallback {`
- L143: `export interface IScrollable {`
- L149: `export interface ISelectable {`
- L156: `export struct AnimState {`
- L161: `export struct WidgetEventSlot {`
- L166: `export struct WidgetValueSlot {`
- L171: `export struct Widget : IStyled {`
- L330: `export fn widget_text_len(text: &const byte) usize {`
- L337: `export fn widget_new(name_ptr: &const byte, type_tag: u32, clickable: bool, focusable: bool) Widget {`
- L355: `export fn widget_bind_callbacks(base: &GuiObj, render_cb: &IGuiRenderCallback, input_cb: &IGuiInputCallback, user_data: &void) &GuiObj {`
- L362: `export fn widget_font(style: &Style) &Font {`
- L370: `export fn widget_text_color(style: &Style, state: WidgetState) Color {`
- L385: `export fn widget_state_bg(style: &Style, state: WidgetState) Color {`
- L402: `export fn widget_inner_rect(widget: &Widget) Rect {`
- L424: `export fn widget_draw_surface(widget: &Widget, ctx: &RenderCtx) Rect {`
- L451: `export fn widget_render_children(base: &GuiObj, ctx: &RenderCtx) void {`
- L461: `export fn widget_measure_text(text: &const byte, font: &Font) i16 {`
- L469: `export fn widget_clone_color(color: Color) Color {`
- L473: `export fn widget_default_font() Font {`

## `gui/widget/btn.uya`

- L36: `export enum BtnVariant {`
- L50: `export struct Button {`

## `gui/widget/canvas.uya`

- L26: `export struct Canvas {`

## `gui/widget/chart.uya`

- L13: `export const MAX_CHART_POINTS: i32 = 256;`
- L15: `export enum ChartType {`
- L43: `export struct Chart {`

## `gui/widget/grid_view.uya`

- L57: `export const MAX_GRID_ITEMS: i32 = 128;`
- L63: `export struct GridView : IContainer {`

## `gui/widget/img.uya`

- L25: `export enum ImageScale {`
- L37: `export struct Image {`

## `gui/widget/lbl.uya`

- L20: `export const MAX_LABEL_LINES: i32 = 8;`
- L22: `export enum TextAlign {`
- L34: `export enum TextOverflow {`
- L51: `export struct Label {`

## `gui/widget/list.uya`

- L24: `export const MAX_LIST_ITEMS: i32 = 1024;`
- L37: `export struct ListView : IContainer, IScrollable, ISelectable {`

## `gui/widget/page.uya`

- L131: `export struct Page : IContainer {`
- L318: `export enum PageTransitionKind {`
- L329: `export interface IPageTransitionCallback {`
- L333: `export struct PageTransitionState {`
- L344: `export const PAGE_STACK_CAPACITY: i32 = 8;`
- L346: `export struct PageNavigator {`
- L544: `export fn page_transition_none() PageTransitionState {`
- L557: `export fn page_navigator_new(viewport: Rect) PageNavigator {`

## `gui/widget/panel.uya`

- L113: `export struct Panel : IContainer {`

## `gui/widget/slider.uya`

- L35: `export struct Slider {`

## `gui/widget/switch.uya`

- L24: `export struct Switch {`

