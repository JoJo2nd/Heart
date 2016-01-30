-- Input mappings
-- Keyboard
input.set_default_action_mapping("F4", "Show/Hide Console")
input.set_default_action_mapping("`", "Open Debug Menu")
input.set_default_action_mapping("Return", "Ok")
input.set_default_action_mapping("Backspace", "Cancel")
input.set_default_action_mapping("F6", "Next Test")
input.set_default_action_mapping("Left Arrow", "Turn Left")
input.set_default_action_mapping("Right Arrow", "Turn Right")
input.set_default_action_mapping("Up Arrow", "Look Up")
input.set_default_action_mapping("Down Arrow", "Look Down")
input.set_default_action_mapping("Mouse X", "Turn Left/Right")
input.set_default_action_mapping("Mouse Y", "Look Up/Down")
input.set_default_action_mapping("Mouse X", "UI X Axis")
input.set_default_action_mapping("Mouse Y", "UI Y Axis")
input.set_default_action_mapping("Mouse Left", "UI Select")

--Pad
input.set_default_action_mapping("guide", "Show/Hide Console")
input.set_default_action_mapping("a", "Ok")
input.set_default_action_mapping("b", "Cancel")
input.set_default_action_mapping("start", "Next Test")
input.set_default_action_mapping("leftx", "Turn Left/Right")
input.set_default_action_mapping("lefty", "Look Up/Down")

t_graph = create_task_graph("ExampleGraph")
tasks = {
    clock_update = t_graph:add_task_to_graph("heart::clock::update"),
    resource_update = t_graph:add_task_to_graph("heart::resource_manager::update"),
    sync = t_graph:add_task_to_graph("heart::SYNC"),
}
t_graph:add_task_order_to_graph(tasks.clock_update, tasks.resource_update, tasks.sync)

t_graph = create_task_graph("RenderTargetTestGraph")
tasks = {
    clock_update = t_graph:add_task_to_graph("heart::clock::update"),
    debug_server_service = t_graph:add_task_to_graph("heart::debug_server::service"),
    res_update = t_graph:add_task_to_graph("heart::resource_manager::update"),
    action_update = t_graph:add_task_to_graph("heart::action_manager::update"),
    sync1 = t_graph:add_task_to_graph("heart::SYNC"),
    vm_update = t_graph:add_task_to_graph("heart::lua_vm::update"),
    sync2 = t_graph:add_task_to_graph("hearat::SYNC"),
    publisher_context = t_graph:add_task_to_graph("heart::event_publisher::update"),
    sync3 = t_graph:add_task_to_graph("heart::SYNC"),
    action_eof = t_graph:add_task_to_graph("heart::action_manager::end_of_frame_update"),
    main_update = t_graph:add_task_to_graph("heart::main_frame_update"),
    main_render = t_graph:add_task_to_graph("heart::main_render_update"),
    debug_menus = t_graph:add_task_to_graph("heart::debug_menus::update"),
    views_init = t_graph:add_task_to_graph("heart::views::begin"),
    views_sort = t_graph:add_task_to_graph("heart::views::sort"),
    views_submit = t_graph:add_task_to_graph("heart::views::submit"),
}
t_graph:add_dependency_to_graph(tasks.clock_update, tasks.debug_server_service, tasks.res_update, tasks.action_update, tasks.views_init)
t_graph:add_dependency_to_graph(tasks.views_init, tasks.main_update)
t_graph:add_dependency_to_graph(tasks.sync1, tasks.main_update)
t_graph:add_dependency_to_graph(tasks.main_update, tasks.publisher_context, tasks.debug_menus)
t_graph:add_dependency_to_graph(tasks.debug_menus, tasks.sync3)
t_graph:add_dependency_to_graph(tasks.sync3, tasks.main_render)
t_graph:add_task_order_to_graph(tasks.res_update, tasks.sync1, tasks.vm_update, tasks.sync2, tasks.publisher_context, tasks.sync3, tasks.action_eof)
t_graph:add_task_order_to_graph(tasks.main_render, tasks.views_sort, tasks.views_submit)