#!/usr/bin/env python3
"""Ensure optional installer does not become a mandatory payload dependency."""
import argparse
import subprocess


def validate_dynamic_output(output):
    if 'Dynamic section' not in output or 'Shared library:' not in output:
        raise ValueError('Missing ELF dynamic dependency evidence')
    if '[libSceAppInstUtil.sprx]' in output:
        raise ValueError('Optional installer must not be a mandatory startup dependency')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('elf')
    parser.add_argument('--readelf', default='llvm-readelf-18')
    args = parser.parse_args()
    result = subprocess.run([args.readelf, '-d', args.elf], check=True,
                            capture_output=True, text=True, timeout=15)
    validate_dynamic_output(result.stdout)
    print('Optional settings installer: no mandatory AppInstUtil dependency')
