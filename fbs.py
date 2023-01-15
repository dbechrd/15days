import glob
import os
import sys

def usage(args):
    print("Unexpected args:")
    for arg in args:
        print(arg)
    print("Usage:")
    print("  fbs.py gen <schema_file_pattern> <output_dir>")
    print("  fbs.py fbb2json <schema_file_pattern> <fbb_dir> <json_dir>")
    print("  fbs.py json2fbb <schema_file_pattern> <json_dir> <fbb_dir>")

def run(cmd):
    print(cmd)
    os.system(cmd)

#for i, arg in enumerate(sys.argv):
#    print(f"[{i}] {arg}")

if (len(sys.argv) < 4):
    usage(sys.argv)
    sys.exit(1)

action = sys.argv[1]

if (action not in ["gen", "json2fbb", "fbb2json"]):
    usage(sys.argv)
    sys.exit(1)

schema_file_pattern = sys.argv[2]

schema_files = glob.glob(schema_file_pattern)

if action == "gen":
    output_dir = sys.argv[3]
    if (len(sys.argv) < 4):
        usage(sys.argv)
        sys.exit(1)

    # --gen-mutable       allow user to edit scalar fields in-place
    # --gen-name-strings  generate string converters for e.g. enums
    # --gen-object-api    allow user to Pack/Unpack to native C++ objects
    # --reflect-names     generate RTTI for field types, names, etc.
    for schema_file in schema_files:
        run(f"flatc --cpp --gen-name-strings -o {output_dir} {schema_file}")

elif action == "fbb2json":
    fbb_dir = sys.argv[3]
    json_dir = sys.argv[4]
    if (len(sys.argv) < 5):
        usage(sys.argv)
        sys.exit(1)

    for schema_file in schema_files:
        schema_name = os.path.splitext(os.path.basename(schema_file))[0]
        fbb_path = os.path.join(fbb_dir, schema_name) + ".fbb"
        json_path = os.path.join(json_dir, schema_name) + ".json"
        print(f"[{schema_name}] {fbb_path} -> {json_path}")
        run(f"flatc --json -o {json_dir} {schema_file} -- {fbb_path}")

elif action == "json2fbb":
    json_dir = sys.argv[3]
    fbb_dir = sys.argv[4]
    if (len(sys.argv) < 5):
        usage(sys.argv)
        sys.exit(1)

    for schema_file in schema_files:
        schema_name = os.path.splitext(os.path.basename(schema_file))[0]
        json_path = os.path.join(json_dir, schema_name) + ".json"
        fbb_path = os.path.join(fbb_dir, schema_name) + ".fbb"
        print(f"[{schema_name}] {json_path} -> {fbb_path}")
        run(f"flatc --binary -o {fbb_dir} {schema_file} {json_path}")

else:
    print(f"Unknown action {action}")
