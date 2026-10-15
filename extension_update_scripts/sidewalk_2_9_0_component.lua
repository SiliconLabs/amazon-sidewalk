local changeset = {}

if slc.component_selected('sidewalk_ble_subghz') then
    -- This upgrade rule is needed because if the project was upgraded from 2.5.0 to 2.6.0,
    -- then sidewalk_ble_subghz is still in the project (remove was not supported by slcu at that time)
    -- Now the component is completely removed.
    table.insert(changeset, {
        ['component'] = 'sidewalk_ble_subghz',
        ['description'] = 'sidewalk_ble_subghz component is removed from the project',
        ['action'] = 'remove',
        ['status'] = 'automatic',
    })
end

if slc.component_selected('sidewalk_board_support') then
    -- This upgrade rule is needed because if the project was upgraded from 2.5.0 to 2.6.0,
    -- then sidewalk_board_support is still in the project (remove was not supported by slcu at that time)
    -- Now the component is completely removed.
    table.insert(changeset, {
        ['component'] = 'sidewalk_board_support',
        ['description'] = 'sidewalk_board_support component is removed from the project',
        ['action'] = 'remove',
        ['status'] = 'automatic',
    })
end

if slc.component_selected('app_button_press') then
    -- This upgrade rule is needed because if the project was upgraded from 2.5.0 to 2.6.0,
    -- then app_button_press is still in the project (remove was not supported by slcu at that time)
    -- Now the component is completely removed.
    table.insert(changeset, {
        ['component'] = 'app_button_press',
        ['description'] = 'app_button_press component is removed from the project',
        ['action'] = 'remove',
        ['status'] = 'automatic',
    })
end

if slc.component_selected('app_log') then
    -- This upgrade rule is needed because if the project was upgraded from 2.7.0 to 2.8.0,
    -- then app_log is still in the project (remove was not supported by slcu at that time)
    -- Now the component is completely removed.
    table.insert(changeset, {
        ['component'] = 'app_log',
        ['description'] = 'app_log component is removed from the project',
        ['action'] = 'remove',
        ['status'] = 'automatic',
    })
end

if slc.component_selected('sidewalk_common') then
    -- Sidewalk Assistant is moved to a new component from the slcp files.
    table.insert(changeset, {
        ['component'] = 'sidewalk_assistant',
        ['description'] = 'sidewalk_assistant component is added to the project',
        ['action'] = 'add',
        ['status'] = 'automatic',
    })
end

return changeset