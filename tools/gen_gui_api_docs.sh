#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
out_file="${1:-"$repo_root/docs/gui_uya_api_reference.md"}"
tmp_file="$(mktemp)"

{
  echo "# UyaGUI API Reference"
  echo
  echo "> 该文件由 \`tools/gen_gui_api_docs.sh\` 从 \`gui/**/*.uya\` 自动生成。"
  echo "> 内容聚焦公开符号索引，详细设计说明请结合 \`docs/gui_uya_*.md\` 系列文档阅读。"
  echo

  while IFS= read -r file; do
    rel_path="${file#"$repo_root/"}"
    matches="$(grep -nE '^export (mc|const|enum|struct|interface|fn|@async_fn fn)' "$file" || true)"
    if [[ -z "$matches" ]]; then
      continue
    fi

    echo "## \`$rel_path\`"
    echo

    while IFS= read -r match; do
      line_no="${match%%:*}"
      decl="${match#*:}"
      decl="$(printf '%s' "$decl" | sed -E 's/[[:space:]]+/ /g; s/[[:space:]]+$//')"
      echo "- L${line_no}: \`${decl}\`"
    done <<< "$matches"

    echo
  done < <(find "$repo_root/gui" -type f -name '*.uya' | sort)
} > "$tmp_file"

mv "$tmp_file" "$out_file"
