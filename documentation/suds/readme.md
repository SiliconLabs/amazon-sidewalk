# Guide to generate D.S.C. documentation locally

This document describes the flow of generating Doxy and SUDS documentation for Sidewalk and gives an explanation of the present files.

## List of folders and files:

- **sld583-sidewalk-services-api**
    - `_docleaf-sld583-sidewalk-services-api.yml`: YAML file containing necessary fields for the API Reference page.
    - `sidewalk-sdk-api.md`: Markdown file containing the structure of the API Reference.
- **sld594-sidewalk-examples**
    - `_docleaf-sld594-sidewalk-examples.yml`: YAML file containing necessary fields for the Sample Application description page.
    - `index.md`: Markdown file containing a short description of the Sample Applications.
    - **INFO**: Please note that the file name MUST be `index.md` for markdown type docleaf.
- `_docspace-sidewalk.yml`: Space in docs.silabs.com, where Sidewalk documentation can be found.
- `build_suds.bat` and `build_suds.sh`: Scripts for Windows and Linux environments for generating Doxy documentation and SUDS document compilation.
- `dsc_release.yml`: Necessary for collecting docLeaf files when serving SUDS documentation.
- `start_sidewalk_sdk_suds_server.bat` and `start_sidewalk_sdk_suds_server.sh`: Scripts for Windows and Linux environments for starting a localhost server to make documentation able to be checked before publishing it to docs.silabs.com.

## Process for generating Documentation locally:

The following step-by-step guide shows the process of generating Doxy documentation and SUDS locally, then starts the SUDS localhost server:

1. Make changes in the required _docLeaf.
2. Call `build_suds.bat` on Windows, and `build_suds.sh` on Linux environment.
3. In some cases, the command `suds compile` fails, so the command shall be executed manually:
    - `suds compile -c ./sld594-sidewalk-examples/_docleaf-sld583-sidewalk-examples.yml --verbose`
    THEN
    - `suds compile -c ./sld583sidewalk-services-api/_docleaf-sld583-sidewalk-services-api.yml --verbose`
4. Call `start_sidewalk_sdk_suds_server.bat` on Windows, and `start_sidewalk_sdk_suds_server.sh` on Linux environment, OR call `suds serve -c dsc_release.yml -o ../output/sidewalk`.
5. Open the link from stdout, and check if your changes took place with the correct format.