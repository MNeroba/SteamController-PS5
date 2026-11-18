#!/usr/bin/env python3
"""Embed exact clean source identity. Dirty desktop builds say unknown."""
import os,re,subprocess
from pathlib import Path
root=Path(__file__).resolve().parents[1]
version=re.search(r'#define STEAMCONTROLLER_VERSION "([^"]+)"',(root/'payload/version.h').read_text())[1]
selected=os.environ.get('SOURCE_SHA','')
requested=os.environ.get('DEV_VERSION',version)
if requested != version:raise SystemExit('Build version must match payload/version.h')
if selected and not re.fullmatch(r'[0-9a-f]{40}',selected):raise SystemExit('Invalid SOURCE_SHA')
if not selected:
    status=subprocess.run(['git','-C',str(root),'status','--porcelain'],capture_output=True,text=True)
    head=subprocess.run(['git','-C',str(root),'rev-parse','HEAD'],capture_output=True,text=True)
    selected=head.stdout.strip() if status.returncode==0 and not status.stdout and head.returncode==0 else 'unknown (uncommitted build)'
(root/'payload/build_info.h').write_text('/* Generated; do not commit. */\n#define STEAMCONTROLLER_SOURCE_SHA "'+selected+'"\n')
