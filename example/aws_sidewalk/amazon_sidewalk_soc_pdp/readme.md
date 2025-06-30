# Readme

## Change RTT IOStream to VCOM IOStream

It is enough to remove the `iostream_rtt` component from the project and recompile it to change the IOStream to VCOM. If both are defined, RTT will be selected by default.