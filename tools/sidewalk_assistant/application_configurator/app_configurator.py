import argparse
import json
import os
import sys

from jinja2 import Environment, FileSystemLoader

parser = argparse.ArgumentParser(add_help=True)
parser.add_argument("-in", "--input", help="Input directory", required=True)
parser.add_argument("-out", "--output", help="Output directory", required=True)
args = parser.parse_args()

############################################
# Constants and file names / extensions
############################################
INPUT_FILE_NAME = "sidewalk.asconf"
PROPERTIES_TEMPLATE_NAME = "sl_sidewalk_properties.h.jinja"
PROPERTIES_OUTPUT_NAME = "sl_sidewalk_properties.h"

SCRIPT_PATH = os.path.dirname(os.path.realpath(__file__))
TEMPLATE_DIR = os.path.join(SCRIPT_PATH, "templates")

env = Environment(loader=FileSystemLoader(TEMPLATE_DIR))
template = env.get_template(PROPERTIES_TEMPLATE_NAME)
macros = []


############################################
# @brief add property to the properties dictionary
############################################
def add_property(pKey, pVal):
    if pVal == None:
        return
    if pVal["value"] == None:
        # silently return for null properties
        return

    # find in dictionary
    if pVal["type"] == "SYMBOL":
        pass
    elif pVal["type"] == "INT":
        pass
    elif pVal["type"] == "STR":
        # concatenate " " to the string
        pVal["value"] = '"' + pVal["value"] + '"'
    else:
        raise ValueError("Invalid property type")

    # add to properties
    macros.append({"key": pKey, "value": pVal["value"]})


############################################
# @brief render the jinja template based on
# the script context
############################################
def render_template(template, output_file, context):
    content = template.render(context)
    out_dir = os.path.dirname(output_file)
    if out_dir and not os.path.exists(out_dir):
        os.makedirs(out_dir, exist_ok=True)
    with open(output_file, "w", encoding="utf-8") as f:
        f.write(content)


############################################
# @brief from the json input file, add new properties
############################################
def generate_properties(asconf_path):
    prop_default_link = find_in_json_file("defaultLinkType", file=asconf_path)
    add_property(
        "SL_SIDEWALK_CONFIGURATOR_PROPERTIES_DEFAULT_LINK_TYPE",
        prop_default_link,
    )

    prop_devicename = find_in_json_file("bleSettings", "devicename", file=asconf_path)
    if prop_devicename is not None:
        if prop_devicename["value"] is None or prop_devicename["value"] == "":
            prop_devicename["value"] = "SL_SIDEWALK"
            prop_devicename["type"] = "STR"
    add_property(
        "SL_SIDEWALK_CONFIGURATOR_PROPERTIES_BLE_DEVICE_NAME",
        prop_devicename,
    )
    prop_ble_output = find_in_json_file("bleSettings", "outputPower", file=asconf_path)
    add_property(
        "SL_SIDEWALK_CONFIGURATOR_PROPERTIES_BLE_OUTPUT_POWER",
        prop_ble_output,
    )

    prop_subghz_output = find_in_json_file(
        "subGHzSettings", "outputPower", file=asconf_path
    )
    add_property(
        "SL_SIDEWALK_CONFIGURATOR_PROPERTIES_SUBGHZ_OUTPUT_POWER",
        prop_subghz_output,
    )


############################################
# @brief find the key in the json file
############################################
def find_in_json_file(*keys, file):
    with open(file, "r", encoding="utf-8") as f:
        data = json.load(f)
        for key in keys:
            if key not in data:
                return None
            data = data[key]
        return data


def main():
    asconf_path = os.path.join(args.input, INPUT_FILE_NAME)
    if not os.path.isfile(asconf_path):
        print(f"Error: missing {asconf_path}", file=sys.stderr)
        sys.exit(1)

    macros.clear()
    generate_properties(asconf_path)

    out_path = os.path.join(args.output, PROPERTIES_OUTPUT_NAME)
    context = {"properties": macros}
    render_template(template, out_path, context)


if __name__ == "__main__":
    main()
