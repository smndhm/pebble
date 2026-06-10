"""Shared Pebble emulator helpers (imported by make_screenshots.py)."""
import sys, os, time, datetime, signal, errno

SITE = '/home/simonduhem/.local/share/uv/tools/pebble-tool/lib/python3.11/site-packages'
sys.path.insert(0, SITE)

_qemu_dir = os.path.expanduser('~/.pebble-sdk/SDKs/4.9.77/toolchain/bin')
if _qemu_dir not in os.environ.get('PATH', ''):
    os.environ['PATH'] = _qemu_dir + os.pathsep + os.environ.get('PATH', '')

from libpebble2.communication import PebbleConnection
from libpebble2.communication.transports.websocket import MessageTargetPhone
from libpebble2.communication.transports.websocket.protocol import WebSocketRelayQemu
from libpebble2.communication.transports.qemu.protocol import QemuPacket, QemuTimeFormat
from libpebble2.services.appmessage import AppMessageService, Int32
from libpebble2.protocol.system import TimeMessage, SetUTC
import pebble_tool.sdk.emulator as emu_mod
from pebble_tool.sdk.emulator import ManagedEmulatorTransport
from pebble_tool.sdk import sdk_manager

BW_PLATFORMS = {'aplite', 'diorite', 'flint'}


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


def set_24h(pebble):
    transport = pebble.transport
    tf = QemuTimeFormat(is_24_hour=True)
    packet = QemuPacket(data=tf)
    packet.serialise()
    transport.send_packet(
        WebSocketRelayQemu(protocol=packet.protocol, data=tf.serialise()),
        target=MessageTargetPhone()
    )
    time.sleep(0.2)


def set_time(pebble, hour, minute, second=0):
    # Use tomorrow as base so the timestamp is always in the future — the
    # Pebble emulator rejects backward time jumps, which happens whenever
    # the target H:M:S is earlier than the current real-world time.
    tomorrow = datetime.datetime.now() + datetime.timedelta(days=1)
    target = tomorrow.replace(hour=hour, minute=minute, second=second, microsecond=0)
    ts = int(target.timestamp())
    tz_offset = -time.altzone if time.localtime(ts).tm_isdst and time.daylight else -time.timezone
    tz_minutes = tz_offset // 60
    tz_name = "UTC%+d" % (tz_minutes // 60)
    pebble.send_packet(TimeMessage(message=SetUTC(unix_time=ts, utc_offset=tz_minutes, tz_name=tz_name)))
    time.sleep(1.0)


def send_appmessage(pebble, app_uuid, msg):
    """Send AppMessage. msg: {int_key: int_value}"""
    svc = AppMessageService(pebble)
    svc.send_message(app_uuid, {k: Int32(v) for k, v in msg.items()})
    svc.shutdown()
    time.sleep(1.5)


def take_screenshot(pebble):
    import argparse
    from pebble_tool.commands.screenshot import ScreenshotCommand
    sc = ScreenshotCommand()
    sc.pebble = pebble
    return sc._grab_processed_image(argparse.Namespace(no_correction=False, scale=1), show_progress=False)


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
