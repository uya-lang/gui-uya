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

## `gui/bench_suite.uya`

- L5: `export fn main() i32 {`

## `gui/benchmarks/core_bench.uya`

- L306: `export fn run_core_bench() i32 {`

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

## `gui/core/event_dispatch.uya`

- L35: `export struct EventDispatcher {`
- L232: `export fn event_dispatcher_new() EventDispatcher {`

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
- L301: `export fn empty_framebuffer_color() Color {`
- L305: `export fn render_ctx_new(fb: FrameBuffer) RenderCtx {`
- L327: `export fn empty_render_ctx() RenderCtx {`
- L331: `export fn default_ctx() RenderCtx {`
- L335: `export fn ctx_pixel(ctx: &RenderCtx, x: i16, y: i16) Color {`

## `gui/render/font.uya`

- L5: `export enum TextAlign {`
- L11: `export struct Glyph {`
- L21: `export struct Font {`
- L50: `export fn font_default() Font {`
- L61: `export fn glyph_for_char(font: &Font, code: u8) Glyph {`
- L73: `export fn text_width(text: &const byte, len: usize, font: &Font) i16 {`
- L81: `export fn draw_text(ctx: &RenderCtx, x: i16, y: i16, text: &const byte, len: usize, font: &Font, color: Color) void {`
- L94: `export fn draw_text_aligned(ctx: &RenderCtx, rect: Rect, text: &const byte, len: usize, font: &Font, color: Color, align: TextAlign) void {`

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

## `gui/tests/test_event.uya`

- L17: `export const TEST_EVENT_MODULE: i32 = 1;`

## `gui/tests/test_event_dispatch.uya`

- L20: `export const TEST_EVENT_DISPATCH_MODULE: i32 = 1;`

## `gui/tests/test_input_dev.uya`

- L25: `export const TEST_INPUT_DEV_MODULE: i32 = 1;`

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

- L27: `export const TEST_RENDER_ASSETS_MODULE: i32 = 1;`

## `gui/tests/test_render_ctx.uya`

- L17: `export const TEST_RENDER_CTX_MODULE: i32 = 1;`

## `gui/tests/test_render_pipeline.uya`

- L23: `export const TEST_RENDER_PIPELINE_MODULE: i32 = 1;`

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

- L22: `export const WIDGET_TYPE_WIDGET: u32 = 0x1000u32;`
- L23: `export const WIDGET_TYPE_BUTTON: u32 = 0x1001u32;`
- L24: `export const WIDGET_TYPE_LABEL: u32 = 0x1002u32;`
- L25: `export const WIDGET_TYPE_IMAGE: u32 = 0x1003u32;`
- L26: `export const WIDGET_TYPE_SLIDER: u32 = 0x1004u32;`
- L27: `export const WIDGET_TYPE_SWITCH: u32 = 0x1005u32;`
- L28: `export const WIDGET_TYPE_PAGE: u32 = 0x1006u32;`
- L29: `export const WIDGET_TYPE_PANEL: u32 = 0x1007u32;`
- L30: `export const WIDGET_TYPE_LIST: u32 = 0x1008u32;`
- L31: `export const WIDGET_TYPE_GRID_VIEW: u32 = 0x1009u32;`
- L32: `export const WIDGET_TYPE_CHART: u32 = 0x100Au32;`
- L33: `export const WIDGET_TYPE_CANVAS: u32 = 0x100Bu32;`
- L95: `export enum WidgetState {`
- L104: `export enum WidgetOrientation {`
- L109: `export interface IStyled {`
- L115: `export interface IWidgetEventCallback {`
- L119: `export interface IWidgetValueCallback {`
- L123: `export interface IScrollable {`
- L129: `export interface ISelectable {`
- L136: `export struct AnimState {`
- L141: `export struct WidgetEventSlot {`
- L146: `export struct WidgetValueSlot {`
- L151: `export struct Widget : IStyled {`
- L310: `export fn widget_text_len(text: &const byte) usize {`
- L317: `export fn widget_new(name_ptr: &const byte, type_tag: u32, clickable: bool, focusable: bool) Widget {`
- L335: `export fn widget_bind_callbacks(base: &GuiObj, render_cb: &IGuiRenderCallback, input_cb: &IGuiInputCallback, user_data: &void) &GuiObj {`
- L342: `export fn widget_font(style: &Style) &Font {`
- L350: `export fn widget_text_color(style: &Style, state: WidgetState) Color {`
- L365: `export fn widget_state_bg(style: &Style, state: WidgetState) Color {`
- L382: `export fn widget_inner_rect(widget: &Widget) Rect {`
- L404: `export fn widget_draw_surface(widget: &Widget, ctx: &RenderCtx) Rect {`
- L431: `export fn widget_render_children(base: &GuiObj, ctx: &RenderCtx) void {`
- L441: `export fn widget_measure_text(text: &const byte, font: &Font) i16 {`
- L449: `export fn widget_clone_color(color: Color) Color {`
- L453: `export fn widget_default_font() Font {`

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

