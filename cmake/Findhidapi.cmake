# Accept either a packaged HIDAPI installation or an explicit SDK root.
find_path(hidapi_INCLUDE_DIR NAMES hidapi.h PATH_SUFFIXES hidapi include/hidapi include)
find_library(hidapi_LIBRARY NAMES hidapi hidapi-hidraw hidapi-libusb PATH_SUFFIXES lib x64)
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(hidapi REQUIRED_VARS hidapi_LIBRARY hidapi_INCLUDE_DIR)
if(hidapi_FOUND AND NOT TARGET hidapi::hidapi)
    add_library(hidapi::hidapi UNKNOWN IMPORTED)
    set_target_properties(hidapi::hidapi PROPERTIES
        IMPORTED_LOCATION "${hidapi_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${hidapi_INCLUDE_DIR}"
        INTERFACE_COMPILE_DEFINITIONS HID_API_NO_EXPORT_DEFINE)
endif()
mark_as_advanced(hidapi_INCLUDE_DIR hidapi_LIBRARY)
