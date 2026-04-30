#!/usr/bin/env python3
import os
import sys
import subprocess
import argparse
import yaml
import shlex

def my_env(env):
    """Return the URL based on the provided environment."""
    urls = {
        'qa': "https://api-suds-cm-qa.dev.silabs.net",
        'stage': "https://api-suds-cm-stage.silabs.net",
        'production': "https://api-suds-cm-prod.silabs.net"
    }
    return urls.get(env, "https://api-suds-cm-prod.silabs.net")

def get_release_yml_files(release_file):
    """
    Check that the YAML release file exists and contains non-empty 
    'docSpaceConfigs' and 'docLeafConfigs' sections.
    
    Returns True if both sections exist and are non-empty lists; else False.
    """
    if not os.path.isfile(release_file):
        sys.stderr.write(f"Error: Release YML file '{release_file}' not found.\n")
        return False
    try:
        with open(release_file, 'r') as f:
            data = yaml.safe_load(f)
    except Exception as e:
        sys.stderr.write(f"Error reading the YAML file '{release_file}': {e}\n")
        return False

    if not isinstance(data, dict):
        sys.stderr.write(f"Error: YAML file '{release_file}' does not contain a valid dictionary.\n")
        return False

    doc_space_configs = data.get('docSpaceConfigs')
    if not doc_space_configs or not isinstance(doc_space_configs, list) or len(doc_space_configs) == 0:
        sys.stderr.write("Error: 'docSpaceConfigs' is missing or empty in the YAML release file.\n")
        return False

    doc_leaf_configs = data.get('docLeafConfigs')
    if not doc_leaf_configs or not isinstance(doc_leaf_configs, list) or len(doc_leaf_configs) == 0:
        sys.stderr.write("Error: 'docLeafConfigs' is missing or empty in the YAML release file.\n")
        return False

    return True

def run_command(command_str):
    """
    Print the nicely formatted command string, then use shlex.split to convert it
    into a command list for subprocess.run.
    """
    print("Running:", command_str)
    command_list = shlex.split(command_str)
    result = subprocess.run(command_list, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    print(result.stdout)
    if result.returncode != 0:
        sys.exit(result.returncode)
    return result.stdout

def push_scratch(env, release_file, dsc_token):
    """Execute the 'upload-scratch' operation."""
    if not get_release_yml_files(release_file):
        sys.exit("Error: Release YAML file check failed.")
    my_path = my_env(env)
    if not dsc_token:
        sys.exit("Error: DSC token not provided.")
    # Build the command string as a single f-string.
    cmd_str = f"suds upload-scratch -t {shlex.quote(dsc_token)} -c {shlex.quote(release_file)} -u {shlex.quote(my_path)} --verbose"
    run_command(cmd_str)

def push_release_preview(ticket, env, release_file, dsc_token):
    """
    Execute the 'upload-release' command for a release preview.

    The release ticket is determined as follows:
      - If the provided ticket parameter is non-empty, that value is used.
      - Otherwise, the ticket is extracted from the YAML file.
    """
    my_path = my_env(env)
    if not ticket:
        sys.exit("Stopping early. No Ticket ID was provided. Please contact DocCurator/DocPublisher for a valid ticket id.")
    if not get_release_yml_files(release_file):
        sys.exit("Error: Release YAML file check failed.")
    if not dsc_token:
        sys.exit("Error: DSC token not provided.")
    cmd_str = f"suds upload-release -t {shlex.quote(dsc_token)} -c {shlex.quote(release_file)} -u {shlex.quote(my_path)} -rt {shlex.quote(ticket)} --verbose"
    run_command(cmd_str)
    print(f"Done with release {ticket}")

def main():
    parser = argparse.ArgumentParser(description="DSC Release Operations")
    subparsers = parser.add_subparsers(dest="command", required=True,
                                       help="Sub-command to run: pushScratch or pushReleasePreview")

    # Subparser for pushScratch.
    parser_scratch = subparsers.add_parser("pushScratch", help="Run the push-scratch operation")
    parser_scratch.add_argument('--env', default='production', help='Target environment (default: production)')
    parser_scratch.add_argument('--release-file', default='./dsc_release.yml', help='Path to release YAML file')
    parser_scratch.add_argument('--dsc-token', required=True, help='DSC token provided from Jenkins credentials')

    # Subparser for pushReleasePreview.
    parser_preview = subparsers.add_parser("pushReleasePreview", help="Run the push-release-preview operation")
    parser_preview.add_argument('ticket', nargs='?', default='', help='Ticket ID (if empty, it will be extracted from the YAML file)')
    parser_preview.add_argument('--env', default='production', help='Target environment (default: production)')
    parser_preview.add_argument('--release-file', default='./dsc_release.yml', help='Path to release YAML file')
    parser_preview.add_argument('--dsc-token', required=True, help='DSC token provided from Jenkins credentials')

    args = parser.parse_args()

    if args.command == "pushScratch":
        push_scratch(env=args.env, release_file=args.release_file, dsc_token=args.dsc_token)
    elif args.command == "pushReleasePreview":
        push_release_preview(args.ticket, env=args.env, release_file=args.release_file, dsc_token=args.dsc_token)
    else:
        parser.print_help()

if __name__ == "__main__":
    main()
