#!/home/simonduhem/.local/share/uv/tools/pebble-tool/bin/python3
"""
Capture screenshots for any Pebble watchface.
Usage: tools/make_screenshots.py <path/to/watchface/screenshots/config.json>

Config format (JSON):

  Simple shots (same for all platforms):
    { "shots": [{"slug": "thin", "time": [0,0,0], "appmessage": {"KEY": value}}, ...] }

  Matrix expansion (color platforms) + bw_shots (B&W platforms):
    {
      "time": [20, 26, 0],
      "matrix": {
        "teams": [{"slug": "france", "team_index": 23, "bg": 43775, "fg": 255, "fg2": 16711680}, ...],
        "modes": [{"slug": "logo", "team_mode": "colors", "DISPLAY_MODE": 0, ...}, ...]
      },
      "bw_shots": [{"slug": "logo", "appmessage": {"DISPLAY_MODE": 0, ...}}, ...]
    }

  team_mode "colors" sends BGCOLOR/FGCOLOR/COLOR_RIGHT from team colors.
  team_mode "index"  sends TEAM_INDEX from team.
"""
import sys, os, time, json, argparse

TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, TOOLS_DIR)
import pebble_emulator as emu

import png
from uuid import UUID


def load_project(project_dir):
    with open(os.path.join(project_dir, 'package.json')) as f:
        pkg = json.load(f)
    pebble = pkg['pebble']
    app_uuid = UUID(pebble['uuid'])
    pbw_path = os.path.join(project_dir, 'build', f"{pkg['name']}.pbw")
    platforms = pebble.get('targetPlatforms', [])

    mk_path = os.path.join(project_dir, 'build', 'js', 'message_keys.json')
    if os.path.exists(mk_path):
        with open(mk_path) as f:
            msg_keys = json.load(f)
    else:
        keys = pebble.get('messageKeys', [])
        msg_keys = {k: i for i, k in enumerate(keys)} if isinstance(keys, list) else keys

    return app_uuid, pbw_path, platforms, msg_keys


def resolve_msg(raw_msg, msg_keys):
    result = {}
    for k, v in raw_msg.items():
        int_key = msg_keys[k] if isinstance(k, str) else int(k)
        int_val = int(v, 16) if isinstance(v, str) and v.startswith('0x') else int(v)
        result[int_key] = int_val
    return result


def expand_matrix(matrix, msg_keys, default_time):
    shots = []
    for team in matrix['teams']:
        for mode in matrix['modes']:
            t = tuple(mode.get('time', default_time))
            mode_msg = {k: v for k, v in mode.items() if k not in ('slug', 'team_mode', 'time')}
            if mode.get('team_mode') == 'index':
                raw = {'TEAM_INDEX': team['team_index'], **mode_msg}
            else:
                raw = {'BGCOLOR': team['bg'], 'FGCOLOR': team['fg'], 'COLOR_RIGHT': team['fg2'], **mode_msg}
            shots.append({
                'slug': f"{team['slug']}/{mode['slug']}",
                'time': t,
                'appmessage': resolve_msg(raw, msg_keys),
            })
    return shots


def build_platform_shots(config, msg_keys, platforms):
    default_time = tuple(config.get('time', [0, 0, 0]))
    platform_shots = {}
    for platform in platforms:
        is_bw = platform in emu.BW_PLATFORMS
        if is_bw and 'bw_shots' in config:
            shots = [{'slug': s['slug'],
                      'time': tuple(s.get('time', default_time)),
                      'appmessage': resolve_msg(s.get('appmessage', {}), msg_keys)}
                     for s in config['bw_shots']]
        elif not is_bw and 'matrix' in config:
            shots = expand_matrix(config['matrix'], msg_keys, default_time)
        else:
            shots = [{'slug': s['slug'],
                      'time': tuple(s.get('time', default_time)),
                      'appmessage': resolve_msg(s.get('appmessage', {}), msg_keys)}
                     for s in config.get('shots', [])]
        platform_shots[platform] = shots
    return platform_shots


def run(platform_shots, app_uuid, pbw_path, out_dir):
    os.makedirs(out_dir, exist_ok=True)
    total = sum(len(shots) for shots in platform_shots.values())
    done = 0

    for platform, shots in platform_shots.items():
        print(f'\n=== {platform} ===')
        pebble = None
        try:
            pebble = emu.connect_emulator(platform)
            from pebble_tool.commands.install import ToolAppInstaller
            ToolAppInstaller(pebble, pbw_path, quiet=True).install()
            time.sleep(1.5)
            emu.set_24h(pebble)

            prev_time = None
            for shot in shots:
                done += 1
                slug, t, msg = shot['slug'], shot['time'], shot.get('appmessage', {})
                print(f'  [{done}/{total}] {platform}/{slug}')

                if t != prev_time:
                    emu.set_time(pebble, *t)
                    prev_time = t

                if msg:
                    emu.send_appmessage(pebble, app_uuid, msg)
                else:
                    time.sleep(0.5)

                image = emu.take_screenshot(pebble)
                out_path = os.path.join(out_dir, platform, f'{slug}.png')
                os.makedirs(os.path.dirname(out_path), exist_ok=True)
                png.from_array(image, mode='RGBA;8').save(out_path)

        except Exception as e:
            import traceback
            print(f'ERROR on {platform}: {e}')
            traceback.print_exc()
        finally:
            emu.close_pebble(pebble)
            emu.shutdown_emulator(platform)
            time.sleep(2.0)


def main():
    parser = argparse.ArgumentParser(description='Capture Pebble watchface screenshots')
    parser.add_argument('config', help='Path to screenshots/config.json')
    args = parser.parse_args()

    config_path = os.path.abspath(args.config)
    screenshots_dir = os.path.dirname(config_path)
    project_dir = os.path.dirname(screenshots_dir)

    with open(config_path) as f:
        config = json.load(f)

    app_uuid, pbw_path, platforms, msg_keys = load_project(project_dir)
    platform_shots = build_platform_shots(config, msg_keys, platforms)
    run(platform_shots, app_uuid, pbw_path, screenshots_dir)
    print(f'\nDone — screenshots in {screenshots_dir}')


if __name__ == '__main__':
    main()
