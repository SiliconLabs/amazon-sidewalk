
local changeset = {}

if slc.is_selected('sidewalk_ble_subghz') then
  -- sidewalk_ble_subghz component is renamed to sidewalk_pal
  table.insert(changeset, {
    ['component'] = 'sidewalk_ble_subghz',
    ['description'] = 'sidewalk_ble_subghz component is renamed to sidewalk_pal',
    ['action'] = 'remove',
    ['status'] = 'automatic',
  })

  table.insert(changeset, {
    ['component'] = 'sidewalk_pal',
    ['description'] = 'sidewalk_pal component is added to the project',
    ['action'] = 'add',
    ['status'] = 'automatic',
  })
end

if slc.is_provided('device_supports_bluetooth') and not slc.is_selected('sidewalk_pdp') then
  -- Sidewalk BLE is a standalone component now. Adding to the project...
  table.insert(changeset, {
    ['component'] = 'sidewalk_ble',
    ['description'] = 'Sidewalk BLE is a standalone component now. Adding to the project...',
    ['action'] = 'add',
    ['status'] = 'automatic',
  })
end

if slc.is_selected('sidewalk_board_support') then
  table.insert(changeset, {
    ['component'] = 'sidewalk_board_support',
    ['description'] = 'sidewalk_board_support component is removed',
    ['action'] = 'remove',
    ['status'] = 'automatic',
  })
end

if slc.is_selected('app_button_press') then
  table.insert(changeset, {
    ['component'] = 'app_button_press',
    ['description'] = "Replace Sidewalk's app_button_press component from GSDK. app_button_press component is removed",
    ['action'] = 'remove',
    ['status'] = 'automatic',
  })

  table.insert(changeset, {
    ['component'] = 'sidewalk_app_button_press',
    ['description'] = 'sidewalk_app_button_press component is added',
    ['action'] = 'add',
    ['status'] = 'automatic',
  })
end

return changeset
