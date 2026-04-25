# GUI Uya Development Setup

## Toolchain

This repository bundles the compiler at `./uya/bin/uya`.
The current bundled version is `v0.9.4`, which satisfies the Phase 0 baseline.

Useful checks:

```bash
./uya/bin/uya --version
make build
make test
make bench
```

## Git Hooks

Enable repository-local hooks:

```bash
make hooks
```

The pre-commit hook formats staged `*.uya` files with `uyaFmt` when it is available and then runs `make test`.
`uyaFmt` is optional for the current Phase 0 workflow.

## VS Code / Cursor

The repository includes:

- `.vscode/settings.json` for editor defaults
- `.vscode/tasks.json` for `build`, `test`, and `bench`
- `.editorconfig` for shared formatting rules

## Cross-Compilation Entry Points

Phase 0 adds the following convenience targets:

```bash
make build-arm
make build-riscv
make build-esp32
```

`build-arm` emits C99 for downstream Cortex-M toolchains, while `build-riscv` and `build-esp32` emit microapp artifacts for the currently supported bare-metal profiles.
