#!/home/simonduhem/.local/share/uv/tools/pebble-tool/bin/python3
"""
Generate store screenshots for fwc26.
6 platforms × 4 teams × 5 modes = 120 screenshots at 20:26.
"""
import sys, os, time, datetime, signal, errno, argparse

SITE = '/home/simonduhem/.local/share/uv/tools/pebble-tool/lib/python3.11/site-packages'
sys.path.insert(0, SITE)

# qemu-pebble must be in PATH for ManagedEmulatorTransport to spawn it
_qemu_dir = os.path.expanduser('~/.pebble-sdk/SDKs/4.9.77/toolchain/bin')
if _qemu_dir not in os.environ.get('PATH', ''):
    os.environ['PATH'] = _qemu_dir + os.pathsep + os.environ.get('PATH', '')

import png
from uuid import UUID

from libpebble2.communication import PebbleConnection
from libpebble2.communication.transports.websocket import MessageTargetPhone
from libpebble2.communication.transports.websocket.protocol import WebSocketRelayQemu
from libpebble2.communication.transports.qemu.protocol import QemuPacket, QemuTimeFormat
from libpebble2.services.appmessage import AppMessageService, Int32
from libpebble2.protocol.system import TimeMessage, SetUTC
import pebble_tool.sdk.emulator as emu_mod
from pebble_tool.sdk.emulator import ManagedEmulatorTransport
from pebble_tool.commands.install import ToolAppInstaller
from pebble_tool.commands.screenshot import ScreenshotCommand
from pebble_tool.sdk import sdk_manager

APP_UUID = UUID('c2edb3a7-a11c-4e91-8c24-c4b87c7649de')
HERE     = os.path.dirname(os.path.abspath(__file__))
PBW_PATH = os.path.join(HERE, 'build', 'fwc26.pbw')
OUT_DIR  = os.path.join(HERE, 'screenshots')

# AppMessage keys (from build/src/message_keys.auto.c)
MK_BGCOLOR      = 10000
MK_FGCOLOR      = 10001
MK_COLOR_RIGHT  = 10002
MK_DISPLAY_MODE = 10003
MK_TEAM_INDEX   = 10004
MK_COL_COLOR    = 10005
MK_LOGO_PIXEL   = 10007
MK_SUB_MODE     = 10008

# basalt first: avoids connection race on re-start
PLATFORMS = ['basalt', 'aplite', 'chalk', 'diorite', 'emery', 'flint']
BW_PLATFORMS = {'aplite', 'diorite', 'flint'}

# (slug, team_index, bg_argb8, fg_argb8, fg2_argb8, col_argb8)
TEAMS = [
    ('france', 23, 0xCB, 0xC3, 0xF0, 0xFF),
    ('usa',    22, 0xCB, 0xC3, 0xC2, 0xF0),
    ('canada', 11, 0xCB, 0xF0, 0xFF, 0xF5),
    ('mexico', 31, 0xCB, 0xC4, 0xF0, 0xFF),
]

# (slug, display_mode, logo_pixel, sub_mode)
COLOR_MODES = [
    ('logo',        0, 0, 0),
    ('logo_pixel',  0, 1, 0),
    ('sup_minutes', 1, 0, 0),
    ('sup_hm',      1, 0, 1),
    ('classic',     2, 0, 0),
]
BW_MODES = [
    ('logo',        0, 0, 0),
    ('sup_minutes', 1, 0, 0),
    ('sup_hm',      1, 0, 1),
    ('classic',     2, 0, 0),
]

def argb8_to_rgb24(a):
    """Convert Pebble ARGB8 color to 24-bit RGB for GColorFromHEX."""
    r = ((a >> 4) & 0x3) * 85
    g = ((a >> 2) & 0x3) * 85
    b = (a & 0x3) * 85
    return (r << 16) | (g << 8) | b

def connect_emulator(platform, retries=3):
    for attempt in range(retries):
        try:
            transport = ManagedEmulatorTransport(platform, sdk_manager.get_current_sdk())
            pebble = PebbleConnection(transport)
            pebble.connect()
            pebble.run_async()
            return pebble
        except Exception as e:
            if attempt < retries - 1:
                print(f'  retry {attempt+1}…')
                time.sleep(3)
            else:
                raise

def set_24h_and_time(pebble):
    """Set 24h mode then time 20:26."""
    transport = pebble.transport

    # Set 24h format via QEMU protocol
    tf = QemuTimeFormat(is_24_hour=True)
    packet = QemuPacket(data=tf)
    packet.serialise()
    transport.send_packet(
        WebSocketRelayQemu(protocol=packet.protocol, data=tf.serialise()),
        target=MessageTargetPhone()
    )
    time.sleep(0.2)

    # Set time to 20:26:00
    now = datetime.datetime.now()
    target = now.replace(hour=20, minute=26, second=0, microsecond=0)
    ts = int(target.timestamp())
    tz_offset = -time.altzone if time.localtime(ts).tm_isdst and time.daylight else -time.timezone
    tz_minutes = tz_offset // 60
    tz_name = "UTC%+d" % (tz_minutes // 60)
    for _ in range(2):
        pebble.send_packet(TimeMessage(message=SetUTC(unix_time=ts, utc_offset=tz_minutes, tz_name=tz_name)))
        time.sleep(0.35)
    time.sleep(0.5)

def send_config(pebble, display_mode, logo_pixel, sub_mode, team_idx, bg, fg, fg2, col):
    svc = AppMessageService(pebble)
    is_sup  = (display_mode == 1)
    has_team = (team_idx >= 0)
    msg = {
        MK_DISPLAY_MODE: Int32(display_mode),
        MK_LOGO_PIXEL:   Int32(logo_pixel),
        MK_SUB_MODE:     Int32(sub_mode),
    }
    if is_sup and has_team:
        msg[MK_TEAM_INDEX] = Int32(team_idx)
    elif has_team:
        msg[MK_BGCOLOR]     = Int32(argb8_to_rgb24(bg))
        msg[MK_FGCOLOR]     = Int32(argb8_to_rgb24(fg))
        msg[MK_COLOR_RIGHT] = Int32(argb8_to_rgb24(fg2))
    svc.send_message(APP_UUID, msg)
    svc.shutdown()
    time.sleep(1.5)

def shutdown_emulator(platform):
    info = emu_mod.get_emulator_info(platform, sdk_manager.get_current_sdk())
    if not info:
        return
    for key in ('qemu', 'pypkjs', 'websockify'):
        pid = info.get(key, {}).get('pid')
        if not pid:
            continue
        try:
            os.kill(pid, signal.SIGTERM)
        except OSError as e:
            if e.errno != errno.ESRCH:
                raise
    emu_mod.update_emulator_info(platform, info['version'], None)

def close_pebble(pebble):
    if not pebble:
        return
    ws = getattr(getattr(pebble, 'transport', None), 'ws', None)
    if ws:
        try:
            ws.close()
        except Exception:
            pass

def main():
    os.makedirs(OUT_DIR, exist_ok=True)
    fake_args = argparse.Namespace(no_correction=False, scale=1)

    color_shots = len(TEAMS) * len(COLOR_MODES)
    bw_shots    = len(BW_MODES)
    total = sum(bw_shots if p in BW_PLATFORMS else color_shots for p in PLATFORMS)
    done  = 0

    for platform in PLATFORMS:
        print(f'\n=== {platform} ===')
        pebble = None
        is_bw  = platform in BW_PLATFORMS
        try:
            pebble = connect_emulator(platform)
            ToolAppInstaller(pebble, PBW_PATH, quiet=True).install()
            time.sleep(1.5)
            set_24h_and_time(pebble)

            sc = ScreenshotCommand()
            sc.pebble = pebble

            team_list = [None] if is_bw else TEAMS
            for team in team_list:
                team_name = team[0] if team else 'bw'
                team_idx  = team[1] if team else -1
                bg, fg, fg2, col = (team[2], team[3], team[4], team[5]) if team else (0,0,0,0)

                for mode_slug, display_mode, logo_pixel, sub_mode in (BW_MODES if is_bw else COLOR_MODES):
                    done += 1
                    label = f'{platform}/{team_name}/{mode_slug}'
                    print(f'  [{done}/{total}] {label}')

                    send_config(pebble, display_mode, logo_pixel, sub_mode,
                                team_idx, bg, fg, fg2, col)

                    image = sc._grab_processed_image(fake_args, show_progress=False)

                    if is_bw:
                        out = os.path.join(OUT_DIR, platform, f'{mode_slug}.png')
                    else:
                        out = os.path.join(OUT_DIR, platform, team_name, f'{mode_slug}.png')
                    os.makedirs(os.path.dirname(out), exist_ok=True)
                    png.from_array(image, mode='RGBA;8').save(out)

        except Exception as e:
            import traceback
            print(f'ERROR on {platform}: {e}')
            traceback.print_exc()
        finally:
            close_pebble(pebble)
            shutdown_emulator(platform)
            time.sleep(2.0)

    print(f'\nDone — screenshots in {OUT_DIR}')

if __name__ == '__main__':
    main()
