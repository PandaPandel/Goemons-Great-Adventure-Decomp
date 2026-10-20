"""asm-differ settings: compare one function of the build with the baserom.

    python <asm-differ>/diff.py -o <function>      (after python canon-decompilation-tools/build_rom.py)

Both images are the DECOMPRESSED ROM (canon-decompilation-tools/nisitenma_archive.py unpack / canon-decompilation-tools/build_rom.py), where every
code overlay sits uncompressed at the offsets in goemonsgreatadv.yaml.
    BINUTILS   directory holding mips64-elf-objdump   (default ../vendor/mips64/bin)
"""

import os

ROOT = os.path.dirname(os.path.abspath(__file__))


def apply(config, args):
    exe = ".exe" if os.name == "nt" else ""
    binutils = os.environ.get("BINUTILS", os.path.join(ROOT, "..", "vendor", "mips64", "bin"))
    config["arch"] = "mips"
    config["baseimg"] = "baserom.us.decompressed.z64"
    config["myimg"] = "build/goemonsgreatadv.decompressed.z64"
    config["mapfile"] = "build/goemonsgreatadv.map"
    config["map_format"] = "gnu"
    config["build_dir"] = "build"
    config["source_directories"] = ["src", "include"]
    config["objdump_executable"] = os.path.abspath(
        os.path.join(binutils, "mips64-elf-objdump" + exe)).replace("\\", "/")
    config["make_command"] = ["python", "canon-decompilation-tools/build_rom.py"]
