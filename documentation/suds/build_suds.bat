@echo off

REM Navigate to the doxygen directory
cd ..\doxygen

REM Remove the out_doxygen directory if it exists
if exist out_doxygen (
    rmdir /s /q out_doxygen
)

REM Run doxygen with the specified configuration file
doxygen sidewalk.doxyfile

REM Navigate back to the suds directory
cd ..\suds

REM Remove the _sdm directory if it exists
if exist _sdm (
    rmdir /s /q _sdm
)

REM Run the suds compile command with the specified configuration file
suds compile -c ./sld583-sidewalk-services-api/_docleaf-sld583-sidewalk-services-api.yml --verbose
suds compile -c ./sld889-sidewalk-sw-components/_docleaf-sld889-sidewalk-sw-components.yml --verbose
suds compile -c ./sld594-sidewalk-sample-applications/_docleaf-sld594-sidewalk-sample-applications.yml --verbose
