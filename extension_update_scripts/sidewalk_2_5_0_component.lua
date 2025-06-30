
local changeset = {}

if slc.is_provided('sl_sidewalk_default_radio_sx1262') and
   slc.is_provided('spidrv')  then -- If spidrv is listed in the slcp, that means the project is using sx1262 radio
  table.insert(changeset, {
    ['component'] = 'sidewalk_subghz_sx1262',
    ['action'] = 'add',
    ['status'] = 'automatic',
    ['description'] = 'Add SX1262 component to the project',
  })
end

if slc.is_provided('sl_sidewalk_default_radio_efr32xgxx') then
  table.insert(changeset, {
    ['component'] = 'sidewalk_subghz_efr32xgxx',
    ['action'] = 'add',
    ['status'] = 'automatic',
    ['description'] = 'Add efr32xgxx rail radio component to the project',
  })
end

if slc.is_selected('sidewalk_common') and not slc.is_selected('sidewalk_pdp') then
  -- Warn user if there is FreeRTOS in the project. PDP is the only app where there is no FreeRTOS
  table.insert(changeset, {
    ['status'] = 'user_verification',
    ['description'] = '!!! WARNING !!!\nDue to the GSDK update MAIN_TASK_STACK_SIZE needs to be increased manually. We suggest 2048 bytes increase. (Project->Properties->C/C++ General->Paths and Symbols->Symbols, both Assembly and GNU C)',
  })
end

return changeset
