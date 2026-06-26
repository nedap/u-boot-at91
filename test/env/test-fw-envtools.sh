#!/bin/sh
# SPDX-License-Identifier: GPL-2.0+
#
# Validate the fw_printenv compatibility options used by Renos diagnostics.

set -eu

SRCTREE="$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)"
WORK="$(mktemp -d "${TMPDIR:-/tmp}/u-boot-at91-fw-envtools.XXXXXX")"
OUT_ARG="${1:-}"
OUT="${OUT_ARG:-${WORK}/build}"
JOBS="${JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)}"
FW_PRINTENV="${FW_PRINTENV:-}"
CLEAN_OUT=0

case "${OUT}" in
/*) ;;
*) OUT="$(pwd)/${OUT}" ;;
esac

if [ -z "${FW_PRINTENV}" ] && [ -n "${OUT_ARG}" ] && [ ! -e "${OUT}" ]; then
	CLEAN_OUT=1
fi

cleanup()
{
	if [ "${CLEAN_OUT}" -eq 1 ]; then
		case "${OUT}" in
		"${WORK}"|"${WORK}"/*) ;;
		*) rm -rf "${OUT}" ;;
		esac
	fi
	rm -rf "${WORK}"
}
trap cleanup EXIT INT TERM

if [ -z "${FW_PRINTENV}" ]; then
	make -C "${SRCTREE}" O="${OUT}" tools-only_config >/dev/null
	make -C "${SRCTREE}" O="${OUT}" -j"${JOBS}" envtools >/dev/null

	FW_PRINTENV="${OUT}/tools/env/fw_printenv"
fi

case "${FW_PRINTENV}" in
/*) ;;
*) FW_PRINTENV="$(pwd)/${FW_PRINTENV}" ;;
esac

if [ ! -x "${FW_PRINTENV}" ]; then
	echo "FAIL: fw_printenv is not executable: ${FW_PRINTENV}" >&2
	exit 1
fi

python3 - "${WORK}" <<'PY'
import binascii
import struct
import sys
from pathlib import Path

root = Path(sys.argv[1])
env_size = 0x2000

def env_data(entries, data_size):
    encoded = [f"{key}={value}".encode() for key, value in entries]
    blob = b"\0".join(encoded) + b"\0\0"
    if len(blob) > data_size:
        raise SystemExit("fixture too large")
    used = sum(len(entry) + 1 for entry in encoded)
    return blob + b"\0" * (data_size - len(blob)), used

# envtools reads the CRC through the host C struct, so these file-backed
# regression fixtures intentionally use host-native uint32_t byte order.
def single(entries):
    data, used = env_data(entries, env_size - 4)
    return struct.pack("=I", binascii.crc32(data) & 0xffffffff) + data, used

def redundant(entries, flag):
    data, used = env_data(entries, env_size - 5)
    return struct.pack("=I", binascii.crc32(data) & 0xffffffff) + bytes([flag]) + data, used

def corrupt_crc(image):
    return bytes([image[0] ^ 0xff]) + image[1:]

single_img, single_used = single([
    ("bootcmd", "run single"),
    ("longvar", "abc def"),
])
setenv_img, _ = single([
    ("bootcmd", "run old"),
    ("longvar", "abc def"),
])
active_img, active_used = redundant([
    ("bootcmd", "run active"),
    ("side", "active"),
], 2)
# File-backed fixtures use MTD_ABSENT, which selects FLAG_INCREMENTAL: higher
# flag wins, so the active page uses 2 and the fallback page uses 1.
fallback_img, fallback_used = redundant([
    ("bootcmd", "run fallback"),
    ("side", "fallback"),
], 1)
slot0_img, slot0_used = redundant([
    ("bootcmd", "run slot0"),
    ("side", "slot0"),
], 1)
slot1_img, slot1_used = redundant([
    ("bootcmd", "run slot1"),
    ("side", "slot1"),
], 2)
bad_fallback_img, _ = redundant([
    ("bootcmd", "badcrc fallback"),
], 1)
bad_active_img, _ = redundant([
    ("bootcmd", "badcrc active"),
], 2)
unterminated_data = b"A" * (env_size - 4)
unterminated_img = (
    struct.pack("=I", binascii.crc32(unterminated_data) & 0xffffffff)
    + unterminated_data
)
malformed_tail_data = (
    b"bootcmd=tail-present\0" +
    b"B" * ((env_size - 4) - len(b"bootcmd=tail-present\0"))
)
malformed_tail_img = (
    struct.pack("=I", binascii.crc32(malformed_tail_data) & 0xffffffff)
    + malformed_tail_data
)
hidden_tail_data = (
    b"\0hidden=1\0\0" +
    b"\0" * ((env_size - 4) - len(b"\0hidden=1\0\0"))
)
hidden_tail_img = (
    struct.pack("=I", binascii.crc32(hidden_tail_data) & 0xffffffff)
    + hidden_tail_data
)

(root / "single.bin").write_bytes(single_img)
(root / "setenv.bin").write_bytes(setenv_img)
(root / "redundant.bin").write_bytes(active_img + fallback_img)
(root / "reversed.bin").write_bytes(slot0_img + slot1_img)
(root / "bad-fallback.bin").write_bytes(active_img + corrupt_crc(bad_fallback_img))
(root / "active-bad.bin").write_bytes(corrupt_crc(slot0_img) + slot1_img)
(root / "both-bad.bin").write_bytes(
    corrupt_crc(bad_active_img) + corrupt_crc(bad_fallback_img)
)
(root / "unterminated.bin").write_bytes(unterminated_img)
(root / "malformed-tail.bin").write_bytes(malformed_tail_img)
(root / "hidden-tail.bin").write_bytes(hidden_tail_img)

(root / "single.config").write_text(
    f"{root / 'single.bin'} 0x0 0x{env_size:x}\n"
)
(root / "setenv.config").write_text(
    f"{root / 'setenv.bin'} 0x0 0x{env_size:x}\n"
)
(root / "redundant.config").write_text(
    f"{root / 'redundant.bin'} 0x0 0x{env_size:x}\n"
    f"{root / 'redundant.bin'} 0x{env_size:x} 0x{env_size:x}\n"
)
(root / "reversed.config").write_text(
    f"{root / 'reversed.bin'} 0x0 0x{env_size:x}\n"
    f"{root / 'reversed.bin'} 0x{env_size:x} 0x{env_size:x}\n"
)
(root / "bad-fallback.config").write_text(
    f"{root / 'bad-fallback.bin'} 0x0 0x{env_size:x}\n"
    f"{root / 'bad-fallback.bin'} 0x{env_size:x} 0x{env_size:x}\n"
)
(root / "active-bad.config").write_text(
    f"{root / 'active-bad.bin'} 0x0 0x{env_size:x}\n"
    f"{root / 'active-bad.bin'} 0x{env_size:x} 0x{env_size:x}\n"
)
(root / "both-bad.config").write_text(
    f"{root / 'both-bad.bin'} 0x0 0x{env_size:x}\n"
    f"{root / 'both-bad.bin'} 0x{env_size:x} 0x{env_size:x}\n"
)
(root / "unterminated.config").write_text(
    f"{root / 'unterminated.bin'} 0x0 0x{env_size:x}\n"
)
(root / "malformed-tail.config").write_text(
    f"{root / 'malformed-tail.bin'} 0x0 0x{env_size:x}\n"
)
(root / "hidden-tail.config").write_text(
    f"{root / 'hidden-tail.bin'} 0x0 0x{env_size:x}\n"
)

(root / "single.used").write_text(f"{single_used}\n")
(root / "active.used").write_text(f"{active_used}\n")
(root / "fallback.used").write_text(f"{fallback_used}\n")
(root / "slot0.used").write_text(f"{slot0_used}\n")
(root / "slot1.used").write_text(f"{slot1_used}\n")
PY

fw()
{
	config="$1"
	shift
	"${FW_PRINTENV}" -l "${WORK}" -c "${config}" "$@"
}

fail()
{
	echo "FAIL: $*" >&2
	exit 1
}

assert_eq()
{
	name="$1"
	want="$2"
	got="$3"
	test "${got}" = "${want}" || fail "${name}: expected '${want}', got '${got}'"
}

single_config="${WORK}/single.config"
setenv_config="${WORK}/setenv.config"
redundant_config="${WORK}/redundant.config"
reversed_config="${WORK}/reversed.config"
bad_fallback_config="${WORK}/bad-fallback.config"
active_bad_config="${WORK}/active-bad.config"
both_bad_config="${WORK}/both-bad.config"
unterminated_config="${WORK}/unterminated.config"
malformed_tail_config="${WORK}/malformed-tail.config"
hidden_tail_config="${WORK}/hidden-tail.config"
fw_setenv="${WORK}/fw_setenv"

ln -sf "${FW_PRINTENV}" "${fw_setenv}"

assert_eq "single variable" "bootcmd=run single" \
	"$(fw "${single_config}" bootcmd)"
assert_eq "single value only" "abc def" \
	"$(fw "${single_config}" -n longvar)"
assert_eq "single used bytes" "$(cat "${WORK}/single.used")" \
	"$(fw "${single_config}" -u "")"
assert_eq "single offset" "0" \
	"$(fw "${single_config}" -o)"

"${fw_setenv}" -l "${WORK}" -c "${setenv_config}" newvar hello ||
	fail "setenv create failed"
assert_eq "setenv created variable" "newvar=hello" \
	"$(fw "${setenv_config}" newvar)"
"${fw_setenv}" -l "${WORK}" -c "${setenv_config}" bootcmd "run new" ||
	fail "setenv overwrite failed"
assert_eq "setenv overwritten variable" "bootcmd=run new" \
	"$(fw "${setenv_config}" bootcmd)"

if fw "${single_config}" -f bootcmd >/dev/null 2>"${WORK}/single-fallback.err"; then
	fail "single fallback unexpectedly succeeded"
fi
grep -q "fallback environment requested without redundant environment" \
	"${WORK}/single-fallback.err" ||
	fail "single fallback error did not explain missing redundant environment"

assert_eq "redundant active variable" "bootcmd=run active" \
	"$(fw "${redundant_config}" bootcmd)"
assert_eq "redundant fallback variable" "bootcmd=run fallback" \
	"$(fw "${redundant_config}" -f bootcmd)"
assert_eq "redundant fallback value only" "run fallback" \
	"$(fw "${redundant_config}" -f -n bootcmd)"
assert_eq "redundant active used bytes" "$(cat "${WORK}/active.used")" \
	"$(fw "${redundant_config}" -u ignored)"
assert_eq "redundant active used bytes without variable" "$(cat "${WORK}/active.used")" \
	"$(fw "${redundant_config}" -u)"
assert_eq "redundant fallback used bytes" "$(cat "${WORK}/fallback.used")" \
	"$(fw "${redundant_config}" -u -f ignored)"
assert_eq "redundant active offset" "0" \
	"$(fw "${redundant_config}" -o)"
assert_eq "redundant fallback offset" "8192" \
	"$(fw "${redundant_config}" -f -o)"
assert_eq "redundant long used bytes without variable" "$(cat "${WORK}/active.used")" \
	"$(fw "${redundant_config}" --used)"
assert_eq "redundant long active offset" "0" \
	"$(fw "${redundant_config}" --offset)"
assert_eq "redundant long fallback offset" "8192" \
	"$(fw "${redundant_config}" --fallback --offset)"
assert_eq "redundant long fallback value only" "run fallback" \
	"$(fw "${redundant_config}" --fallback --noheader bootcmd)"
if fw "${redundant_config}" -u -o >/dev/null 2>"${WORK}/used-offset.err"; then
	fail "combined used and offset unexpectedly succeeded"
fi
grep -q "cannot be used together" "${WORK}/used-offset.err" ||
	fail "combined used and offset error did not explain invalid option mix"

assert_eq "reversed active variable" "bootcmd=run slot1" \
	"$(fw "${reversed_config}" bootcmd)"
assert_eq "reversed fallback variable" "bootcmd=run slot0" \
	"$(fw "${reversed_config}" -f bootcmd)"
assert_eq "reversed active used bytes" "$(cat "${WORK}/slot1.used")" \
	"$(fw "${reversed_config}" -u ignored)"
assert_eq "reversed fallback used bytes" "$(cat "${WORK}/slot0.used")" \
	"$(fw "${reversed_config}" -f -u ignored)"
assert_eq "reversed active offset" "8192" \
	"$(fw "${reversed_config}" -o)"
assert_eq "reversed fallback offset" "0" \
	"$(fw "${reversed_config}" -f -o)"

assert_eq "bad fallback active variable" "bootcmd=run active" \
	"$(fw "${bad_fallback_config}" bootcmd)"
assert_eq "bad fallback offset remains available" "8192" \
	"$(fw "${bad_fallback_config}" -f -o)"
if fw "${bad_fallback_config}" -f -u -o >/dev/null 2>"${WORK}/bad-fallback-used-offset.err"; then
	fail "bad-CRC fallback used+offset unexpectedly succeeded"
fi
grep -q "cannot be used together" "${WORK}/bad-fallback-used-offset.err" ||
	fail "bad-CRC fallback used+offset error did not explain invalid option mix"
if fw "${bad_fallback_config}" -f bootcmd >/dev/null 2>"${WORK}/bad-fallback.err"; then
	fail "bad-CRC fallback unexpectedly succeeded"
fi
grep -q "fallback environment has bad CRC" "${WORK}/bad-fallback.err" ||
	fail "bad-CRC fallback error did not explain fallback CRC"

assert_eq "active-bad valid active variable" "bootcmd=run slot1" \
	"$(fw "${active_bad_config}" bootcmd)"
assert_eq "active-bad fallback offset remains available" "0" \
	"$(fw "${active_bad_config}" -f -o)"
if fw "${active_bad_config}" -f bootcmd >/dev/null 2>"${WORK}/active-bad.err"; then
	fail "active-bad fallback unexpectedly succeeded"
fi
grep -q "fallback environment has bad CRC" "${WORK}/active-bad.err" ||
	fail "active-bad fallback error did not explain fallback CRC"

if fw "${both_bad_config}" -f bootcmd >/dev/null 2>"${WORK}/both-bad.err"; then
	fail "both-bad-CRC fallback unexpectedly succeeded"
fi
grep -q "fallback environment has bad CRC" "${WORK}/both-bad.err" ||
	fail "both-bad-CRC fallback error did not explain fallback CRC"

if fw "${unterminated_config}" -u ignored >/dev/null 2>"${WORK}/unterminated.err"; then
	fail "unterminated environment used-size unexpectedly succeeded"
fi
grep -q "environment not terminated" "${WORK}/unterminated.err" ||
	fail "unterminated environment error did not explain termination"

if fw "${unterminated_config}" bootcmd >/dev/null 2>"${WORK}/unterminated-lookup.err"; then
	fail "unterminated environment lookup unexpectedly succeeded"
fi
grep -q "environment not terminated" "${WORK}/unterminated-lookup.err" ||
	fail "unterminated lookup error did not explain termination"

if fw "${unterminated_config}" >/dev/null 2>"${WORK}/unterminated-print.err"; then
	fail "unterminated environment print unexpectedly succeeded"
fi
grep -q "environment not terminated" "${WORK}/unterminated-print.err" ||
	fail "unterminated print error did not explain termination"

if fw "${malformed_tail_config}" bootcmd >/dev/null 2>"${WORK}/malformed-tail-lookup.err"; then
	fail "malformed-tail environment lookup unexpectedly succeeded"
fi
grep -q "environment not terminated" "${WORK}/malformed-tail-lookup.err" ||
	fail "malformed-tail lookup error did not explain termination"
assert_eq "malformed-tail offset remains available" "0" \
	"$(fw "${malformed_tail_config}" -o)"

if "${fw_setenv}" -l "${WORK}" -c "${malformed_tail_config}" bootcmd new \
	>/dev/null 2>"${WORK}/malformed-tail-setenv.err"; then
	fail "malformed-tail setenv unexpectedly succeeded"
fi
grep -q "environment not terminated" "${WORK}/malformed-tail-setenv.err" ||
	fail "malformed-tail setenv error did not explain termination"

if fw "${hidden_tail_config}" hidden >/dev/null 2>"${WORK}/hidden-tail-lookup.err"; then
	fail "hidden-tail environment lookup unexpectedly succeeded"
fi
grep -q "environment not terminated" "${WORK}/hidden-tail-lookup.err" ||
	fail "hidden-tail lookup error did not explain termination"

if "${fw_setenv}" -l "${WORK}" -c "${hidden_tail_config}" foo bar \
	>/dev/null 2>"${WORK}/hidden-tail-setenv.err"; then
	fail "hidden-tail setenv unexpectedly succeeded"
fi
grep -q "environment not terminated" "${WORK}/hidden-tail-setenv.err" ||
	fail "hidden-tail setenv error did not explain termination"

echo "fw envtools compatibility tests: OK"
