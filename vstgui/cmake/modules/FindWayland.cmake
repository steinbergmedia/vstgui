#************************************************************************************************
#
# Wayland Server Delegate
#
# Copyright (c) 2023 CCL Software Licensing GmbH. All Rights Reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# - Redistributions of source code must retain the above copyright notice,
#   this list of conditions and the following disclaimer.
# - Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
# - Neither the name of the wayland-server-delegate project nor the names of its
#   contributors may be used to endorse or promote products derived from this
#   software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS",
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# Filename    : FindWayland.cmake
# Description : Find Wayland libraries and generate protocol headers
#
#[*****************************************************************************************[.rst:
# FindWayland
# -------
#
# Finds wayland libraries and includes.
#
# Targets
# ^^^^^^^
#
# This module provides the following targets, if found:
#
# ``wayland-client``
# ``wayland-server``
# ``wayland-egl``
# ``wayland-protocols``
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This will define the following variables:
#
# ``WAYLAND_FOUND``
#   True if wayland libraries have been found.
# ``WAYLAND_INCLUDE_DIRS``
#   Wayland include directories.
# ``WAYLAND_LIBRARIES``
#   Wayland libraries.
# ``WAYLAND_DEFINITIONS``
#   Wayland compiler definitions.
#]**********************************************************************************************]

find_package (PkgConfig)
pkg_check_modules (PKG_WAYLAND QUIET wayland-client)

set (WAYLAND_DEFINITIONS ${PKG_WAYLAND_CFLAGS})
set (WAYLAND_VERSION ${PKG_WAYLAND_VERSION})

# Find wayland libraries / includes
# When cross-compiling, these libraries and headers need to be found for the target architecture.
find_path (WAYLAND_CLIENT_INCLUDE_DIR NAMES wayland-client.h HINTS ${PKG_WAYLAND_INCLUDE_DIRS} ONLY_CMAKE_FIND_ROOT_PATH)
mark_as_advanced (WAYLAND_CLIENT_INCLUDE_DIR)

if ("client" IN_LIST Wayland_FIND_COMPONENTS)
	find_library (WAYLAND_CLIENT_LIBRARIES NAMES wayland-client HINTS ${PKG_WAYLAND_LIBRARY_DIRS})
	find_library (WAYLAND_CURSOR_LIBRARIES NAMES wayland-cursor HINTS ${PKG_WAYLAND_LIBRARY_DIRS})
endif ()

if ("egl" IN_LIST Wayland_FIND_COMPONENTS)
	find_library (WAYLAND_EGL_LIBRARIES NAMES wayland-egl HINTS ${PKG_WAYLAND_LIBRARY_DIRS})
endif ()

if ("server" IN_LIST Wayland_FIND_COMPONENTS)
	find_library (WAYLAND_SERVER_LIBRARIES NAMES wayland-server HINTS ${PKG_WAYLAND_LIBRARY_DIRS})
endif ()

if ("protocols" IN_LIST Wayland_FIND_COMPONENTS)

	# Find wayland-scanner
	# When cross-compiling, this program needs to be found for the host architecture. However, we need to make sure that the version matches the library version on the target system.
	find_program (WAYLAND_SCANNER NAMES "wayland-scanner.${WAYLAND_VERSION}")
	if (NOT WAYLAND_SCANNER)
		find_program (WAYLAND_SCANNER NAMES wayland-scanner)
	endif ()
	mark_as_advanced (WAYLAND_SCANNER)

	# Generate extra protocol headers

	find_path (WAYLAND_PROTOCOLS_BASEDIR NAMES "stable/xdg-shell/xdg-shell.xml" HINTS "/usr/share/wayland-protocols")
	mark_as_advanced (WAYLAND_PROTOCOLS_BASEDIR)

	list (APPEND WAYLAND_PROTOCOLS
		"stable/xdg-shell/xdg-shell.xml"
		"stable/linux-dmabuf/linux-dmabuf-v1.xml"
		"unstable/xdg-decoration/xdg-decoration-unstable-v1.xml"
	)
	list (REMOVE_DUPLICATES WAYLAND_PROTOCOLS)

	set (WAYLAND_PROTOCOLS_DIR "${CMAKE_CURRENT_BINARY_DIR}/wayland-protocols")
	file (MAKE_DIRECTORY ${WAYLAND_PROTOCOLS_DIR})

	foreach (protocol ${WAYLAND_PROTOCOLS})

		get_filename_component (protocol_name "${protocol}" NAME_WLE)

		# Generate client header
		set (header "${WAYLAND_PROTOCOLS_DIR}/${protocol_name}-client-protocol.h")
		if (NOT EXISTS "${WAYLAND_PROTOCOLS_BASEDIR}/${protocol}")
			message (WARNING "Unknown Wayland protocol: ${protocol_name}")
			file (WRITE "${header}" "")
			continue ()
		endif ()

		execute_process(
			COMMAND
				/bin/sh -c "${WAYLAND_SCANNER} client-header < ${WAYLAND_PROTOCOLS_BASEDIR}/${protocol} > \"${header}\""
		)
	
#		add_custom_command (OUTPUT ${header}
#			COMMAND /bin/sh -c "${WAYLAND_SCANNER} client-header < ${WAYLAND_PROTOCOLS_BASEDIR}/${protocol} > \"${header}\""
#			DEPENDS ${WAYLAND_PROTOCOLS_BASEDIR}/${protocol}
#			VERBATIM USES_TERMINAL
#		)
		list (APPEND protocol_headers ${header})

		# Generate server header
		set (header "${WAYLAND_PROTOCOLS_DIR}/${protocol_name}-server-protocol.h")

		execute_process(
			COMMAND
				/bin/sh -c "${WAYLAND_SCANNER} server-header < ${WAYLAND_PROTOCOLS_BASEDIR}/${protocol} > \"${header}\""
		)

#		add_custom_command (OUTPUT ${header}
#			COMMAND /bin/sh -c "${WAYLAND_SCANNER} server-header < ${WAYLAND_PROTOCOLS_BASEDIR}/${protocol} > \"${header}\""
#			DEPENDS ${WAYLAND_PROTOCOLS_BASEDIR}/${protocol}
#			VERBATIM USES_TERMINAL
#		)
		list (APPEND protocol_headers ${header})

		# Generate source file
		set (sourcefile "${WAYLAND_PROTOCOLS_DIR}/${protocol_name}-protocol.c")
		execute_process(
			COMMAND
				/bin/sh -c "${WAYLAND_SCANNER} private-code < ${WAYLAND_PROTOCOLS_BASEDIR}/${protocol} > \"${sourcefile}\""
		)
#		add_custom_command (OUTPUT ${sourcefile}
#			COMMAND /bin/sh -c "${WAYLAND_SCANNER} private-code < ${WAYLAND_PROTOCOLS_BASEDIR}/${protocol} > \"${sourcefile}\""
#			DEPENDS ${WAYLAND_PROTOCOLS_BASEDIR}/${protocol}
#			VERBATIM USES_TERMINAL
#		)
		list (APPEND protocol_source_files "${sourcefile}")
	endforeach ()

	add_library (vstgui_wayland_protocols OBJECT ${protocol_headers} ${protocol_source_files} ${CMAKE_CURRENT_LIST_FILE})

	set_target_properties (vstgui_wayland_protocols PROPERTIES
		USE_FOLDERS ON
		FOLDER libs
	)

endif ()

# Set result variables
set (WAYLAND_LIBRARIES ${WAYLAND_CLIENT_LIBRARIES} ${WAYLAND_CURSOR_LIBRARIES} ${WAYLAND_EGL_LIBRARIES} ${WAYLAND_SERVER_LIBRARIES})

set (WAYLAND_INCLUDE_DIRS ${WAYLAND_CLIENT_INCLUDE_DIR} ${WAYLAND_PROTOCOLS_DIR})
list (REMOVE_DUPLICATES WAYLAND_INCLUDE_DIRS)

if (TARGET vstgui_wayland_protocols)
	list (APPEND WAYLAND_LIBRARIES vstgui_wayland_protocols)
	target_include_directories (vstgui_wayland_protocols PUBLIC ${WAYLAND_INCLUDE_DIRS})
endif ()

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (Wayland
	FOUND_VAR WAYLAND_FOUND
	REQUIRED_VARS WAYLAND_LIBRARIES WAYLAND_INCLUDE_DIRS
	VERSION_VAR WAYLAND_VERSION
)
