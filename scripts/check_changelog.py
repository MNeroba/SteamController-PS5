#!/usr/bin/env python3
"""Fail closed on missing/ambiguous two-category release changelogs."""
import argparse,re
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CATEGORIES=('Bug Fixes','New Features / Improvements')
VERSION=re.compile(r'v\d+\.\d+\.\d+(?:-dev\.\d+)?')
def entry(text,version):
    if not VERSION.fullmatch(version):raise ValueError('Invalid changelog version')
    headings=list(re.finditer(r'^## (v[^\s]+)\s*$',text,re.M))
    names=[h[1] for h in headings]
    if len(names)!=len(set(names)):raise ValueError('Duplicate changelog versions')
    if version not in names:raise ValueError('Missing changelog entry: '+version)
    index=names.index(version);start=headings[index].end();end=headings[index+1].start() if index+1<len(headings) else len(text)
    content=text[start:end]
    categories=list(re.finditer(r'^### ([^\n]+)$',content,re.M))
    if tuple(h[1] for h in categories)!=CATEGORIES:raise ValueError('Exactly two ordered changelog categories required')
    result=[];seen=set()
    for i,h in enumerate(categories):
        body=content[h.end():categories[i+1].start() if i+1<len(categories) else len(content)].strip()
        if body!='None.':
            lines=body.splitlines()
            if not lines or any(not line.startswith('- ') or len(line)<4 for line in lines):raise ValueError('Use concise bullets or None.')
            for line in lines:
                if line in seen:raise ValueError('Duplicate change across categories')
                seen.add(line)
        result.append('### '+h[1]+'\n\n'+body)
    return '\n\n'.join(result)+'\n'
def validate(version,root=ROOT):
    text=(root/'CHANGELOG.md').read_text()
    # Every historical entry obeys the format, not just the selected one.
    for name in re.findall(r'^## (v[^\s]+)\s*$',text,re.M):entry(text,name)
    defined=re.search(r'#define STEAMCONTROLLER_VERSION "([^"]+)"',(root/'payload/version.h').read_text())
    if not defined or defined[1]!=version:raise ValueError('Changelog/build/release version mismatch')
    return entry(text,version)
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--version',required=True);p.add_argument('--output',type=Path);a=p.parse_args()
    changes=validate(a.version)
    if a.output:a.output.write_text(changes)
    else:print(changes)
