#!/usr/bin/env python3
# This file is part of Desktop App Toolkit,
# a set of libraries for developing nice desktop applications.
#
# For license and copyright information please follow this link:
# https://github.com/desktop-app/legal/blob/master/LEGAL
"""Self-test of the Material overrides and icon variants of codegen_style.

  run_test.py CODEGEN_STYLE [--upstream UPSTREAM_CODEGEN_STYLE] [--update]
      Generates the modules in input/, compares them with expected/ and
      checks that every case in errors/ fails with its error code.
      With --upstream it also compares the classic output with a
      codegen_style built from the upstream sources.
      With --update it rewrites expected/ from the generated files.

  run_test.py --compare UPSTREAM_DIR MATERIAL_DIR
      Compares two directories of generated styles, for example the
      'gen/styles' folders of a build with upstream codegen and of one
      with this codegen. Modules without Material changes must be byte
      identical, the others must assign the upstream values in the
      classic branch.

PNG masks are compared by size and pixels, because their bytes depend
on the PNG encoder of the Qt build.
"""

import argparse
import difflib
import hashlib
import os
import re
import struct
import subprocess
import sys
import tempfile
import zlib

HERE = os.path.dirname(os.path.abspath(__file__))
INPUT = os.path.join(HERE, 'input')
EXPECTED = os.path.join(HERE, 'expected')
ERRORS = os.path.join(HERE, 'errors')

MODULES = [
    'test_base.style',
    'test_user.style',
    'test_icons.style',
    'test_plain.style',
]

# Case folder, input file, expected error code.
ERROR_CASES = [
    ('unknown_key', 'input.style', 810),
    ('included_key', 'input.style', 810),
    ('type_mismatch', 'input.style', 802),
    ('unknown_field', 'input.style', 803),
    ('conflict', 'input.style', 811),
    ('struct_semicolon', 'input.style', 204),
    ('incomplete_icon', 'input.style', 812),
    ('overrides_module', 'input.material.style', 813),
]

DESIGN_INCLUDE = '#include "ui/style/style_core_design.h"'


def decode_png(data):
    pos, idat, header = 8, b'', None
    while pos < len(data):
        length, = struct.unpack('>I', data[pos:pos + 4])
        kind = data[pos + 4:pos + 8]
        body = data[pos + 8:pos + 8 + length]
        if kind == b'IHDR':
            header = struct.unpack('>IIBBBBB', body)
        elif kind == b'IDAT':
            idat += body
        pos += 12 + length
    width, height, depth, color, _, _, interlace = header
    if depth != 8 or interlace or color not in (0, 2, 4, 6):
        raise ValueError('unsupported PNG mask format')
    channels = {0: 1, 2: 3, 4: 2, 6: 4}[color]
    raw = zlib.decompress(idat)
    stride = width * channels
    previous, offset, pixels = bytearray(stride), 0, b''
    for _ in range(height):
        kind = raw[offset]
        line = bytearray(raw[offset + 1:offset + 1 + stride])
        offset += 1 + stride
        for x in range(stride):
            a = line[x - channels] if x >= channels else 0
            b = previous[x]
            c = previous[x - channels] if x >= channels else 0
            if kind == 1:
                line[x] = (line[x] + a) & 0xFF
            elif kind == 2:
                line[x] = (line[x] + b) & 0xFF
            elif kind == 3:
                line[x] = (line[x] + (a + b) // 2) & 0xFF
            elif kind == 4:
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                line[x] = (line[x] + (
                    a if pa <= pb and pa <= pc else b if pb <= pc else c
                )) & 0xFF
        for x in range(width):
            pixel = line[x * channels:(x + 1) * channels]
            pixels += bytes(pixel[:3]) if channels >= 3 else bytes(
                [pixel[0]] * 3)
        previous = line
    return width, height, pixels


ARRAY = re.compile(r'(const uchar iconMask\d+Data\[\] = )(\{[^}]*\})')


def normalize(text):
    def replace(match):
        data = bytes(
            int(value, 16)
            for value in re.findall(r'0x([0-9a-f]{2})', match.group(2)))
        if not data.startswith(b'\x89PNG\r\n\x1a\n'):
            return match.group(0)
        width, height, pixels = decode_png(data)
        digest = hashlib.sha1(pixels).hexdigest()[:16]
        return '%s{ PNG %dx%d %s }' % (match.group(1), width, height, digest)
    return ARRAY.sub(replace, text)


def read(path):
    with open(path, 'r', encoding='utf-8', newline='') as f:
        return f.read()


def generate(codegen, root, inputs, out):
    os.makedirs(out, exist_ok=True)
    args = [
        codegen,
        '-I' + root,
        '-o' + out,
        '-t' + os.path.join(out, 'test_style'),
        '-w' + root,
    ] + [os.path.join(root, name) for name in inputs]
    # The working folder is the first include path, keep it empty.
    return subprocess.run(args, cwd=out, capture_output=True, text=True)


def generated_names():
    names = []
    for module in MODULES:
        base = 'style_' + module.split('.')[0]
        names += [base + '.h', base + '.cpp']
    return names


def check_expected(out, update):
    failed = False
    for name in generated_names():
        path = os.path.join(out, name)
        if not os.path.exists(path):
            print('MISSING ' + name)
            failed = True
            continue
        actual = normalize(read(path))
        expected_path = os.path.join(EXPECTED, name)
        if update:
            with open(expected_path, 'w', encoding='utf-8', newline='') as f:
                f.write(actual)
            continue
        expected = read(expected_path)
        if actual != expected:
            failed = True
            print('DIFF ' + name)
            sys.stdout.writelines(difflib.unified_diff(
                expected.splitlines(True),
                actual.splitlines(True),
                'expected/' + name,
                'generated/' + name))
    return not failed


def check_errors(codegen, temp):
    failed = False
    for folder, name, code in ERROR_CASES:
        root = os.path.join(ERRORS, folder)
        result = generate(codegen, root, [name], os.path.join(temp, folder))
        marker = 'error %d:' % code
        if result.returncode == 0 or marker not in result.stderr:
            failed = True
            print('ERROR CASE %s: expected "%s", exit code %d, output:'
                % (folder, marker, result.returncode))
            print(result.stderr)
    return not failed


def classic_assignments(text):
    """Assignments in init_*() outside the Material branches."""
    result, branch = {}, None
    for line in text.splitlines():
        if line == '\tif (style::IsMaterial()) {':
            branch = 'material'
        elif line == '\t} else {':
            branch = 'classic'
        elif line == '\t}':
            branch = None
        else:
            match = re.match(r'^\t\t?_(\w+) = (.*);$', line)
            if match and branch != 'material':
                result[match.group(1)] = match.group(2)
    return result


def header_constants(text):
    return dict(re.findall(r'^constexpr \w+ (\w+) = (.*);$', text, re.M))


def source_constants(text):
    return dict(re.findall(r'^(?:int|double) _(\w+) = (.*);$', text, re.M))


def compare_dirs(upstream, material, names=None):
    """Checks that the classic output of 'material' matches 'upstream'."""
    if names is None:
        names = sorted(
            name for name in os.listdir(upstream)
            if name.endswith('.h') or name.endswith('.cpp'))
        extra = sorted(
            set(os.listdir(material)) - set(os.listdir(upstream)))
        extra = [n for n in extra if n.endswith('.h') or n.endswith('.cpp')]
        if extra:
            print('NOT IN UPSTREAM: ' + ', '.join(extra))
            return False
    failed, changed = False, 0
    for name in names:
        if not name.endswith('.cpp'):
            continue
        base = name[:-len('.cpp')]
        old_cpp = read(os.path.join(upstream, name))
        new_cpp = read(os.path.join(material, name))
        old_h = read(os.path.join(upstream, base + '.h'))
        new_h = read(os.path.join(material, base + '.h'))
        if DESIGN_INCLUDE not in new_cpp:
            if old_cpp != new_cpp or old_h != new_h:
                failed = True
                print('CHANGED WITHOUT MATERIAL VALUES: ' + base)
            continue
        changed += 1
        old_values = classic_assignments(old_cpp)
        new_values = classic_assignments(new_cpp)
        for key, value in old_values.items():
            if new_values.get(key) != value:
                failed = True
                print('%s: classic %s is %s, upstream %s'
                    % (base, key, new_values.get(key), value))
        new_constants = header_constants(new_h)
        new_variables = source_constants(new_cpp)
        for key, value in header_constants(old_h).items():
            if new_constants.get(key, new_variables.get(key)) != value:
                failed = True
                print('%s: classic constant %s differs from upstream %s'
                    % (base, key, value))
    print('%d modules with Material values, %d compared'
        % (changed, len([n for n in names if n.endswith('.cpp')])))
    return not failed


def main():
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('codegen', nargs='?')
    parser.add_argument('--upstream')
    parser.add_argument('--update', action='store_true')
    parser.add_argument('--compare', nargs=2, metavar=('UPSTREAM', 'MATERIAL'))
    args = parser.parse_args()

    if args.compare:
        return 0 if compare_dirs(*args.compare) else 1
    if not args.codegen:
        parser.error('the path to codegen_style is required')

    with tempfile.TemporaryDirectory() as temp:
        out = os.path.join(temp, 'material')
        result = generate(args.codegen, INPUT, MODULES, out)
        if result.returncode != 0:
            print('codegen_style failed:')
            print(result.stderr)
            return 1
        ok = check_expected(out, args.update)
        ok = check_errors(args.codegen, os.path.join(temp, 'errors')) and ok
        if args.upstream:
            upstream = os.path.join(temp, 'upstream')
            result = generate(args.upstream, INPUT, MODULES, upstream)
            if result.returncode != 0:
                print('upstream codegen_style failed:')
                print(result.stderr)
                return 1
            ok = compare_dirs(upstream, out, generated_names()) and ok
    print('OK' if ok else 'FAILED')
    return 0 if ok else 1


if __name__ == '__main__':
    sys.exit(main())
