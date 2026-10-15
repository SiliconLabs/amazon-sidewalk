#!/bin/bash

# Navigate to the doxygen directory
cd ../doxygen

# Remove the out_doxygen directory if it exists
if [ -d "out_doxygen" ]; then
    rm -rf out_doxygen
fi

# Run doxygen with the specified configuration file
doxygen sidewalk.doxyfile

# Navigate back to the suds directory
cd ../suds

# Remove the _sdm directory if it exists
if [ -d "_sdm" ]; then
    rm -rf _sdm
fi

# Run the suds compile command with the specified configuration file
suds compile -c ./sld583-sidewalk-services-api/_docleaf-sld583-sidewalk-services-api.yml --verbose
suds compile -c ./sld889-sidewalk-sw-components/_docleaf-sld889-sidewalk-sw-components.yml --verbose
suds compile -c ./sld594-sidewalk-sample-applications/_docleaf-sld594-sidewalk-sample-applications.yml --verbose
