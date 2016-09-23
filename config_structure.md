# Config file standard

Object output:
  "mode" => xcb_mode
  "x" => xcb_x
  "y" => xcb_y
  "output" => xcb_output
  "EDID" => edid

Object crtc:
  "ctrc" => xcb_crtc
  "outputs" => Array<output>

Obejct Screen:
  "width" => screen width in px
  "height" => screen height in px
  "width_mm" => screen width in mm
  "height_mm" => screen height in mm
  "crtcs" => Array<crtc>

Object mode
  "screen" => Array<Screen>

Object NamedMode
  "name" => name (defaults to empty string)
  "mode" => mode

Object MonitorSetup
  "connectedEDIDs" => Array<string>
  "modes" => Array<NamedMode>

Object root
  "Known modes" => Array<MonitorSetup>
