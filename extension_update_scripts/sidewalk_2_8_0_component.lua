local changeset = {}

if slc.is_selected('app_log') then
    -- app_log component is renamed to sidewalk_app_log
    table.insert(changeset, {
        ['component'] = 'sidewalk_app_log',
        ['description'] = 'app_log component is renamed to sidewalk_app_log. sidewalk_app_log component is added to the project',
        ['action'] = 'add',
        ['status'] = 'automatic',
    })
end

return changeset