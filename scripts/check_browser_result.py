#!/usr/bin/env python3
"""Require the full browser result emitted by the pinned Playwright CLI."""
import json
import sys
from pathlib import Path


def find_pass(value):
    if isinstance(value, dict):
        if value.get('result') == 'PASS' and type(value.get('checks')) is int:
            return value
        children = value.values()
    elif isinstance(value, list):
        children = value
    elif isinstance(value, str):
        try:
            return find_pass(json.loads(value))
        except (ValueError, TypeError):
            return None
    else:
        return None
    for child in children:
        result = find_pass(child)
        if result:
            return result
    return None


def validate(value):
    result = find_pass(value)
    if result is None or result['checks'] < 181:
        raise ValueError('Browser regression must report PASS with at least 181 checks')
    return result


if __name__ == '__main__':
    result = validate(json.loads(Path(sys.argv[1]).read_text()))
    print(f"Remap browser regression: PASS ({result['checks']} checks)")
