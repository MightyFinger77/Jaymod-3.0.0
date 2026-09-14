# One-shot: bake published EnhMod samples into src/game/enh/enh_defaults.cpp
import pathlib

root = pathlib.Path(r"C:\projects\JaymodFork\3.1.0")
src = root / "dist" / "enhmod"
out = root / "src" / "game" / "enh" / "enh_defaults.cpp"

admin = """# enhmod_admin.db
# One block per admin. GUID is 32 hex chars (PunkBuster / ET cl_guid).
# Empty flags = use flags from that level in enhmod_level.db
#
# [admin]
# name  = PlayerName
# guid  = 0123456789ABCDEF0123456789ABCDEF
# level = 999
# flags =
"""


def byte_array(name, data):
    lines = ["static const unsigned char %s[] = {" % name]
    row = []
    for i, b in enumerate(data):
        if isinstance(b, str):
            b = ord(b)
        row.append("%d" % b)
        if len(row) == 16:
            lines.append("\t" + ", ".join(row) + ",")
            row = []
    if row:
        lines.append("\t" + ", ".join(row) + ",")
    lines.append("\t0")
    lines.append("};")
    return "\n".join(lines)


files = [
    ("kDefXml", (src / "ModEnhConfig.xml").read_bytes()),
    ("kDefCommands", (src / "enhmod_commands.db").read_bytes()),
    ("kDefLevels", (src / "enhmod_level.db").read_bytes()),
    ("kDefZones", (src / "enhmod_antirush.db").read_bytes()),
    ("kDefForce", (src / "forcecvarfile.cfg").read_bytes()),
    ("kDefFlags", (src / "commands_flags.txt").read_bytes()),
    ("kDefAdmin", admin.replace("\n", "\r\n").encode("ascii")),
]

body = [
    '#include <game/enh/enh_defaults.h>',
    "",
]
for name, data in files:
    body.append(byte_array(name, data))
    body.append("")

body.append("const char* EnhDef_Xml() { return (const char*)kDefXml; }")
body.append("const char* EnhDef_Commands() { return (const char*)kDefCommands; }")
body.append("const char* EnhDef_Levels() { return (const char*)kDefLevels; }")
body.append("const char* EnhDef_Zones() { return (const char*)kDefZones; }")
body.append("const char* EnhDef_Force() { return (const char*)kDefForce; }")
body.append("const char* EnhDef_Flags() { return (const char*)kDefFlags; }")
body.append("const char* EnhDef_Admin() { return (const char*)kDefAdmin; }")
body.append("")

out.write_text("\n".join(body), encoding="ascii")
print("wrote", out, "bytes", out.stat().st_size)
