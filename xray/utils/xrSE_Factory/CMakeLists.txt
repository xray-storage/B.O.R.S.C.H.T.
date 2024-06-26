cmake_minimum_required(VERSION 3.15)
project(xrSE_Factory LANGUAGES C CXX)

add_library(${PROJECT_NAME} SHARED)
target_compile_definitions(${PROJECT_NAME} PRIVATE XRSE_FACTORY_EXPORTS)
target_include_directories(${PROJECT_NAME} PRIVATE
	.
	../../xrServerEntities)
target_precompile_headers(${PROJECT_NAME} PRIVATE stdafx.h)
target_sources(${PROJECT_NAME} PRIVATE
	ai_space.cpp
	ai_space.h
	ai_space_inline.h
	alife_artefact_order.cpp
	alife_artefact_order.h
	bone.h
	defines.h
	NET_Compressor.h
	object_item_script_inline.h
	properties_list_helper_script.cpp
	script_debugger_utils.cpp
	script_debugger_utils.h
	script_file.cpp
	script_file.h
	script_properties_list_helper.cpp
	script_properties_list_helper.h
	script_space.h
	script_value.h
	script_value_inline.h
	script_value_wrapper.h
	script_value_wrapper_inline.h
	stdafx.cpp
	stdafx.h
	xr_LuaAllocator.c
	xr_LuaAllocator.h
	xrSE_Factory.cpp
	xrSE_Factory.h
	xrSE_Factory_import_export.h
	xrUIXmlParser.cpp
	xrUIXmlParser.h
	../../xrServerEntities/alife_human_brain.cpp
	../../xrServerEntities/alife_human_brain.h
	../../xrServerEntities/alife_human_brain_inline.h
	../../xrServerEntities/alife_monster_brain.cpp
	../../xrServerEntities/alife_monster_brain.h
	../../xrServerEntities/alife_monster_brain_inline.h
	../../xrServerEntities/alife_space.cpp
	../../xrServerEntities/alife_space.h
	../../xrServerEntities/character_info.cpp
	../../xrServerEntities/character_info.h
	../../xrServerEntities/character_info_defs.h
	../../xrServerEntities/gametype_chooser.cpp
	../../xrServerEntities/gametype_chooser.h
	../../xrServerEntities/object_factory.cpp
	../../xrServerEntities/object_factory.h
	../../xrServerEntities/object_factory_impl.h
	../../xrServerEntities/object_factory_inline.h
	../../xrServerEntities/object_factory_register.cpp
	../../xrServerEntities/object_factory_script.cpp
	../../xrServerEntities/object_factory_space.h
	../../xrServerEntities/object_item_script.cpp
	../../xrServerEntities/object_item_script.h
	../../xrServerEntities/object_item_single.h
	../../xrServerEntities/PHNetState.cpp
	../../xrServerEntities/PHNetState.h
	../../xrServerEntities/PHSynchronize.cpp
	../../xrServerEntities/PHSynchronize.h
	../../xrServerEntities/script_callStack.cpp
	../../xrServerEntities/script_callStack.h
	../../xrServerEntities/script_debugger.cpp
	../../xrServerEntities/script_debugger.h
	../../xrServerEntities/script_debugger_messages.h
	../../xrServerEntities/script_debugger_threads.cpp
	../../xrServerEntities/script_debugger_threads.h
	../../xrServerEntities/script_engine.cpp
	../../xrServerEntities/script_engine.h
	../../xrServerEntities/script_engine_export.cpp
	../../xrServerEntities/script_engine_export.h
	../../xrServerEntities/script_engine_inline.h
	../../xrServerEntities/script_engine_script.cpp
	../../xrServerEntities/script_engine_space.h
	../../xrServerEntities/script_fcolor.h
	../../xrServerEntities/script_fcolor_script.cpp
	../../xrServerEntities/script_flags.h
	../../xrServerEntities/script_flags_script.cpp
	../../xrServerEntities/script_fmatrix.h
	../../xrServerEntities/script_fmatrix_script.cpp
	../../xrServerEntities/script_fvector.h
	../../xrServerEntities/script_fvector_script.cpp
	../../xrServerEntities/script_ini_file.cpp
	../../xrServerEntities/script_ini_file.h
	../../xrServerEntities/script_ini_file_inline.h
	../../xrServerEntities/script_ini_file_script.cpp
	../../xrServerEntities/script_lua_helper.cpp
	../../xrServerEntities/script_lua_helper.h
	../../xrServerEntities/script_net_packet.h
	../../xrServerEntities/script_net_packet_script.cpp
	../../xrServerEntities/script_reader.h
	../../xrServerEntities/script_reader_script.cpp
	../../xrServerEntities/script_rtoken_list.h
	../../xrServerEntities/script_rtoken_list_inline.h
	../../xrServerEntities/script_rtoken_list_script.cpp
	../../xrServerEntities/script_sound_type.h
	../../xrServerEntities/script_sound_type_script.cpp
	../../xrServerEntities/script_space_forward.h
	../../xrServerEntities/script_stack_tracker.cpp
	../../xrServerEntities/script_stack_tracker.h
	../../xrServerEntities/script_stack_tracker_inline.h
	../../xrServerEntities/script_storage.cpp
	../../xrServerEntities/script_storage.h
	../../xrServerEntities/script_storage_inline.h
	../../xrServerEntities/script_storage_space.h
	../../xrServerEntities/script_token_list.cpp
	../../xrServerEntities/script_token_list.h
	../../xrServerEntities/script_token_list_inline.h
	../../xrServerEntities/script_token_list_script.cpp
	../../xrServerEntities/specific_character.cpp
	../../xrServerEntities/specific_character.h
	../../xrServerEntities/xrServer_Factory.cpp
	../../xrServerEntities/xrServer_Object_Base.cpp
	../../xrServerEntities/xrServer_Object_Base.h
	../../xrServerEntities/xrServer_Objects.cpp
	../../xrServerEntities/xrServer_Objects.h
	../../xrServerEntities/xrServer_Objects_Abstract.cpp
	../../xrServerEntities/xrServer_Objects_Abstract.h
	../../xrServerEntities/xrServer_Objects_ALife.cpp
	../../xrServerEntities/xrServer_Objects_ALife.h
	../../xrServerEntities/xrServer_Objects_ALife_All.h
	../../xrServerEntities/xrServer_Objects_ALife_Items.cpp
	../../xrServerEntities/xrServer_Objects_ALife_Items.h
	../../xrServerEntities/xrServer_Objects_ALife_Items_script.cpp
	../../xrServerEntities/xrServer_Objects_ALife_Items_script2.cpp
	../../xrServerEntities/xrServer_Objects_ALife_Items_script3.cpp
	../../xrServerEntities/xrServer_Objects_ALife_Monsters.cpp
	../../xrServerEntities/xrServer_Objects_ALife_Monsters.h
	../../xrServerEntities/xrServer_Objects_ALife_Monsters_script.cpp
	../../xrServerEntities/xrServer_Objects_ALife_Monsters_script2.cpp
	../../xrServerEntities/xrserver_objects_alife_monsters_script3.cpp
	../../xrServerEntities/xrServer_Objects_ALife_Monsters_script4.cpp
	../../xrServerEntities/xrServer_Objects_ALife_script.cpp
	../../xrServerEntities/xrServer_Objects_ALife_script2.cpp
	../../xrServerEntities/xrServer_Objects_ALife_script3.cpp
	../../xrServerEntities/xrServer_Objects_script.cpp
	../../xrServerEntities/xrServer_Objects_script2.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE Xray::Core Xray::XMLParser Xray::Sound Luabind::Luabind Imdexlib::Imdexlib lua51.lib shlwapi.lib)
add_library(Xray::SE_Factory ALIAS ${PROJECT_NAME})
