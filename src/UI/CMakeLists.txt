SET(SOURCES screens/ui_input.c
    screens/ui_monitor.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c)

add_library(ui ${SOURCES})
