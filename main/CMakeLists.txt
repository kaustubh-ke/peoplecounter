# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_REQUIRES 
	spiffs
	esp32-idf-sqlite3
	)
set(COMPONENT_PRIV_REQUIRES nvs_flash )
set(COMPONENT_SRCS "main.c" "dwell.c")
set(COMPONENT_ADD_INCLUDEDIRS "include")

register_component()
