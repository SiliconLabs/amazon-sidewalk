
local changeset = {}

if slc.is_selected('sidewalk_pdp') then
  -- In case of pdp app, we need to decrease the SL_STACK_SIZE
  table.insert(changeset, {
    ['option'] = 'SL_STACK_SIZE',
    ['value'] = '2048',
    ['status'] = 'automatic',
    ['description'] = 'PDP application SL_STACK_SIZE decreased to 2048 bytes',
  })
end

if slc.is_selected('sidewalk_qualification_support') then
  table.insert(changeset, {
    ['option'] = 'configMINIMAL_STACK_SIZE',
    ['value'] = '160',
    ['status'] = 'automatic',
    ['description'] = 'Sidewalk Qualification Support configMINIMAL_STACK_SIZE decreased to 160 bytes',
  })
end

return changeset
