#!/usr/bin/env python3
"""Sync readme.md content referenced in a docleaf menu into local md/ files."""

from __future__ import annotations

import argparse
import re
import sys
from dataclasses import dataclass
from pathlib import Path


MENU_ITEM_PATTERN = re.compile(r"^\s*-\s*(.+?):\s*(.+?)\s*$")

REWRITE_RULES_SW_COMPONENTS = (
    (
        "](../../suds/sld583-sidewalk-services-api/sidewalk-sdk-api.md#ble-adaptation)",
        "](/amazon-sidewalk/{browser-docspace-version}/sidewalk-services-api/sidewalk-sdk-ble-adapter)",
    ),
    (
        "](../../suds/sld583-sidewalk-services-api/sidewalk-sdk-api.md#sub-ghz-interface)",
        "](/amazon-sidewalk/{browser-docspace-version}/sidewalk-services-api/sid-pal-radio-subghz-ifc)",
    ),
    (
        "](../../suds/sld583-sidewalk-services-api/sidewalk-sdk-api.md#swi-interface)",
        "](/amazon-sidewalk/{browser-docspace-version}/sidewalk-services-api/sid-pal-swi-ifc)",
    ),
    (
        "](../../suds/sld583-sidewalk-services-api/sidewalk-sdk-api.md#security-and-crypto-apis)",
        "](/amazon-sidewalk/{browser-docspace-version}/sidewalk-services-api/sid-pal-crypto-ifc-support)",
    ),
)


@dataclass(frozen=True)
class MenuItem:
    title: str
    source_relpath: str


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Copy markdown files referenced in docleaf menu items into the "
            "docleaf md/ folder using normalized filename from title."
        )
    )
    parser.add_argument(
        "--docleaf",
        required=True,
        help="Path to _docleaf-*.yml",
    )
    parser.add_argument(
        "--mode",
        choices=("sw-components", "sample-applications"),
        required=True,
        help="Mode controls rewrite rules.",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print planned changes only, do not write files.",
    )
    return parser.parse_args()


def normalize_title_to_filename(title: str) -> str:
    lowered = title.lower().strip()
    hyphenated = lowered.replace(" ", "-")
    sanitized = re.sub(r"[^a-z0-9-]", "", hyphenated)
    collapsed = re.sub(r"-{2,}", "-", sanitized).strip("-")
    if not collapsed:
        raise ValueError(f"Cannot derive filename from title: {title!r}")
    return f"{collapsed}.md"


def parse_menu_items(docleaf_path: Path) -> list[MenuItem]:
    content = docleaf_path.read_text(encoding="utf-8")
    items: list[MenuItem] = []
    for line in content.splitlines():
        match = MENU_ITEM_PATTERN.match(line)
        if not match:
            continue
        title, source_relpath = match.groups()
        items.append(MenuItem(title=title.strip(), source_relpath=source_relpath.strip()))
    if not items:
        raise ValueError(f"No menu items found in {docleaf_path}")
    return items


def apply_rewrites(content: str, mode: str) -> tuple[str, int]:
    rules = REWRITE_RULES_SW_COMPONENTS if mode == "sw-components" else ()
    total_replacements = 0
    updated = content
    for old, new in rules:
        count = updated.count(old)
        if count:
            total_replacements += count
            updated = updated.replace(old, new)
    return updated, total_replacements


def sync_docleaf(docleaf_path: Path, mode: str, dry_run: bool) -> int:
    md_dir = docleaf_path.parent / "md"
    md_dir.mkdir(parents=True, exist_ok=True)

    menu_items = parse_menu_items(docleaf_path)
    copied_count = 0
    rewrite_count = 0
    created_count = 0
    updated_count = 0

    print(f"[sync] docleaf={docleaf_path}")
    print(f"[sync] mode={mode}")
    print(f"[sync] dry_run={dry_run}")
    print(f"[sync] items={len(menu_items)}")

    for item in menu_items:
        src_path = (docleaf_path.parent / item.source_relpath).resolve()
        if not src_path.exists():
            raise FileNotFoundError(f"Missing source file: {src_path}")

        filename = normalize_title_to_filename(item.title)
        dest_path = md_dir / filename

        source_content = src_path.read_text(encoding="utf-8")
        rendered, replacements = apply_rewrites(source_content, mode=mode)

        old_content = dest_path.read_text(encoding="utf-8") if dest_path.exists() else None
        status = "unchanged"
        if old_content is None:
            status = "create"
            created_count += 1
        elif old_content != rendered:
            status = "update"
            updated_count += 1

        rewrite_count += replacements
        if status in {"create", "update"}:
            copied_count += 1
            if not dry_run:
                dest_path.write_text(rendered, encoding="utf-8")

        print(
            f"[item] {status:<8} title={item.title!r} src={src_path} dest={dest_path} rewrites={replacements}"
        )

    print(
        "[summary] copied_or_updated={} created={} updated={} rewrites={}".format(
            copied_count,
            created_count,
            updated_count,
            rewrite_count,
        )
    )
    return 0


def main() -> int:
    args = parse_args()
    docleaf_path = Path(args.docleaf).resolve()
    if not docleaf_path.exists():
        print(f"Docleaf not found: {docleaf_path}", file=sys.stderr)
        return 1
    return sync_docleaf(docleaf_path=docleaf_path, mode=args.mode, dry_run=args.dry_run)


if __name__ == "__main__":
    raise SystemExit(main())
