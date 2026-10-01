"""Exercise the build hook using temporary, synthetic credentials only."""
import contextlib
import io
import json
import runpy
import stat
import tempfile
import unittest
from pathlib import Path


SCRIPT = Path(__file__).resolve().parents[1] / "scripts" / "wifi_config.py"


class BuildEnvironment:
    def __init__(self, root):
        self.paths = {"$PROJECT_DIR": str(root), "$BUILD_DIR": str(root / "build")}
        self.includes = []

    def subst(self, value):
        return self.paths[value]

    def Append(self, CPPPATH):
        self.includes.extend(CPPPATH)


class WiFiConfigTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        (self.root / "secrets").mkdir()
        self.source = self.root / "secrets" / "wifi.json"
        self.header = self.root / "build" / "private" / "WiFiCredentials.h"

    def run_hook(self):
        env = BuildEnvironment(self.root)
        capture = io.StringIO()
        with contextlib.redirect_stdout(capture), contextlib.redirect_stderr(capture):
            runpy.run_path(str(SCRIPT), init_globals={"env": env, "Import": lambda _: None})
        self.assertEqual(capture.getvalue(), "")
        self.assertEqual(env.includes, [str(self.header.parent)])

    def write_config(self, value):
        self.source.write_text(json.dumps(value), encoding="utf-8")

    def test_utf8_and_cpp_special_characters_stay_byte_exact(self):
        config = {"ssid": 'test-شبکه"\\\n', "password": 'fixture-"\\\n-password'}
        self.write_config(config)
        self.run_hook()
        content = self.header.read_text()
        for key in config:
            encoded = "".join("\\x%02x" % byte for byte in config[key].encode("utf-8"))
            self.assertIn(f'WIFI_{key.upper()}[] = "{encoded}";', content)
            self.assertNotIn(config[key], content)

    def test_permissions_repaired_and_changes_regenerate_header(self):
        self.write_config({"ssid": "fixture-network", "password": "fixture-password"})
        self.header.parent.mkdir(parents=True)
        self.header.write_text("stale")
        self.source.chmod(0o644)
        self.header.parent.chmod(0o755)
        self.header.chmod(0o644)
        self.run_hook()
        self.assertEqual(stat.S_IMODE(self.source.stat().st_mode), 0o600)
        self.assertEqual(stat.S_IMODE(self.header.stat().st_mode), 0o600)
        self.assertEqual(stat.S_IMODE(self.header.parent.stat().st_mode), 0o700)
        self.assertEqual(stat.S_IMODE(self.header.parent.parent.stat().st_mode), 0o700)
        first = self.header.read_text()
        self.write_config({"ssid": "fixture-network", "password": "changed-fixture"})
        self.run_hook()
        self.assertNotEqual(first, self.header.read_text())
        modified = self.header.stat().st_mtime_ns
        self.run_hook()
        self.assertEqual(modified, self.header.stat().st_mtime_ns)

    def test_empty_config_supports_offline_build(self):
        self.write_config({"ssid": "", "password": ""})
        self.run_hook()
        self.assertIn('WIFI_SSID[] = "";', self.header.read_text())

    def test_invalid_configs_fail_without_exposing_values(self):
        invalid = [
            '{"fixture-sensitive":',
            [],
            {"ssid": "fixture-sensitive"},
            {"ssid": 123, "password": "fixture-sensitive"},
            {"ssid": "fixture-sensitive\0", "password": ""},
            {"ssid": "fixture-sensitive\ud800", "password": ""},
            {"ssid": "x" * 33, "password": "fixture-sensitive"},
            {"ssid": "fixture-sensitive", "password": "x" * 65},
            {"ssid": "", "password": "fixture-sensitive"},
            {"ssid": "fixture-sensitive", "password": "", "extra": "value"},
        ]
        for value in invalid:
            with self.subTest(value_type=type(value).__name__):
                if isinstance(value, str):
                    self.source.write_text(value)
                else:
                    self.write_config(value)
                with self.assertRaises(SystemExit) as error:
                    self.run_hook()
                self.assertNotIn("fixture-sensitive", str(error.exception))
                self.assertFalse(self.header.exists())

    def test_missing_file_fails_with_actionable_message(self):
        with self.assertRaises(SystemExit) as error:
            self.run_hook()
        self.assertIn("secrets/wifi.json", str(error.exception))


if __name__ == "__main__":
    unittest.main()
