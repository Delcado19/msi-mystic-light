# Accept either a packaged HIDAPI installation or an explicit SDK root.
find_path(hidapi_INCLUDE_DIR NAMES hidapi.h PATH_SUFFIXES hidapi include/hidapi include)
find_library(hidapi_LIBRARY NAMES hidapi hidapi-hidraw hidapi-libusb PATH_SUFFIXES lib x64)
include(FindPackageHandleStandardArgs)
if(WIN32)
    get_filename_component(hidapi_LIBRARY_DIR "${hidapi_LIBRARY}" DIRECTORY)
    find_file(HIDAPI_RUNTIME_DLL NAMES hidapi.dll HINTS "${hidapi_LIBRARY_DIR}" "${hidapi_LIBRARY_DIR}/../bin")
    find_package_handle_standard_args(hidapi REQUIRED_VARS hidapi_LIBRARY hidapi_INCLUDE_DIR HIDAPI_RUNTIME_DLL)
else()
    find_package_handle_standard_args(hidapi REQUIRED_VARS hidapi_LIBRARY hidapi_INCLUDE_DIR)
endif()
if(hidapi_FOUND AND NOT TARGET hidapi::hidapi)
    if(WIN32)
        # A .lib import library is not the runtime: model both so TARGET_RUNTIME_DLLS deploys HIDAPI transitively.
        add_library(hidapi::hidapi SHARED IMPORTED)
        set_target_properties(hidapi::hidapi PROPERTIES
            IMPORTED_IMPLIB "${hidapi_LIBRARY}" IMPORTED_LOCATION "${HIDAPI_RUNTIME_DLL}")
    else()
        add_library(hidapi::hidapi UNKNOWN IMPORTED)
        set_target_properties(hidapi::hidapi PROPERTIES IMPORTED_LOCATION "${hidapi_LIBRARY}")
    endif()
    set_target_properties(hidapi::hidapi PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${hidapi_INCLUDE_DIR}"
        INTERFACE_COMPILE_DEFINITIONS HID_API_NO_EXPORT_DEFINE)
endif()
mark_as_advanced(hidapi_INCLUDE_DIR hidapi_LIBRARY)
