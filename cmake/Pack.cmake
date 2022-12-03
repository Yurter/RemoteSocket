#------------------------------------------------------------------------------------------------------
# Pack program to Installer
#------------------------------------------------------------------------------------------------------
macro(Pack)
    # http://www.cmake.org/Wiki/CMake:CPackConfiguration
    set(CPACK_PACKAGE_NAME                  ${USER_PROJECT_NAME})
    set(CPACK_PACKAGE_VENDOR                ${ENTERPRISE_NAME})
    set(CPACK_PACKAGE_CONTACT               ${ENTERPRISE_NAME})
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY   ${USER_PROJECT_NAME})
    set(CPACK_PACKAGE_VERSION_MAJOR         ${PROJECT_VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR         ${PROJECT_VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH         ${PROJECT_VERSION_PATCH})
    set(CPACK_PACKAGE_VERSION
        "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}")

    if("${CPACK_OUTPUT_FILE_PREFIX}" STREQUAL "")
        set(CPACK_OUTPUT_FILE_PREFIX packages) # subfolder of build dir(CMAKE_BINARY_DIR)
    endif()
    set(CPACK_SOURCE_STRIP_FILES            TRUE)

    # Default generator
    if(NOT CPACK_GENERATOR)
        if(WIN32)
            set(CPACK_GENERATOR "NSIS")
        elseif(UNIX)
            if(MCBC)
                set(CPACK_GENERATOR "RPM")
            else()
                set(CPACK_GENERATOR "DEB")
            endif()
        endif()
    endif()

    set(CPACK_PACKAGE_INSTALL_DIRECTORY ${USER_PROJECT_NAME})
    set(CPACK_MONOLITHIC_INSTALL        1)

    set(PLATFORM_TOKEN "")
    if(WIN32)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(PLATFORM_TOKEN             "Win64")
        elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
            set(PLATFORM_TOKEN             "Win32")
        endif()
    else()
        if (ASTRA_LINUX_1_6)
            set(PLATFORM_TOKEN             "Astra16")
        elseif(ASTRA_LINUX_1_5)
            set(PLATFORM_TOKEN             "Astra15")
        elseif(ASTRA_LINUX_1_3)
            set(PLATFORM_TOKEN             "Astra13")
        endif()
    endif()

#    set(CPACK_PACKAGE_FILE_NAME "Setup${USER_PROJECT_NAME}-${CPACK_PACKAGE_VERSION}-${PLATFORM_TOKEN}")
    set(CPACK_PACKAGE_FILE_NAME "Package-RemoteSocket-${CMAKE_BUILD_TYPE}")

    if(NOT "${STC_PROJECT_NAME}" STREQUAL "")
        set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-${STC_PROJECT_NAME}")
    endif()

    # Add CI branch name for not "release/*"
    if (NOT "${CI_BRANCH}" STREQUAL "" AND NOT "${CI_BRANCH}" MATCHES "^release/.*")
        string(MAKE_C_IDENTIFIER "${CI_BRANCH}" CI_BRANCH)
        string(REPLACE "_" "-" CI_BRANCH "${CI_BRANCH}")
        set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-${CI_BRANCH}")
    endif()

    message(STATUS "CPACK PACKAGE FILE NAME: ${CPACK_PACKAGE_FILE_NAME}")

    if(CPACK_GENERATOR MATCHES "NSIS")
        set(CPACK_NSIS_DISPLAY_NAME                     ${USER_PROJECT_NAME})
        set(CPACK_NSIS_HELP_LINK                        "")
        set(CPACK_NSIS_CONTACT                          "")
        set(CPACK_NSIS_MODIFY_PATH                      "ON")
        set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL  true)
        set(CPACK_NSIS_EXECUTABLES_DIRECTORY            ".")
        set(CPACK_NSIS_MUI_FINISHPAGE_RUN               ${USER_PROJECT_NAME})
    endif()

    if(CPACK_GENERATOR MATCHES "DEB")
        set(CPACK_DEBIAN_PACKAGE_MAINTAINER             ${CPACK_PACKAGE_VENDOR} )
        set(CPACK_DEBIAN_ARCHITECTURE                   ${CMAKE_SYSTEM_PROCESSOR} )
        set(CPACK_DEBIAN_PACKAGE_SECTION                "electronics")
        set(CPACK_PACKAGING_INSTALL_PREFIX              "/usr/local/${USER_PROJECT_NAME}")
        set(CPACK_${CPACK_GENERATOR}_COMPONENT_INSTALL  OFF)
        set(CPACK_COMPONENTS_IGNORE_GROUPS ON)
        set(CMAKE_SKIP_BUILD_RPATH  FALSE)
        set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
        set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
#        set(CPACK_DEBIAN_PACKAGE_DEPENDS                "libfftw3-3, pulseaudio")
#        if (ASTRA_LINUX_1_6)
#            set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, sqlite, libprotobuf10")
#        else()
#            set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libprotobuf7, libqt5sql5-sqlite")
#        endif()

        if (ASTRA_LINUX_1_6)
            # Зависмости для 1.6 - используем сситемную Qt, заивисимости собираем автоматически
            set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
            set(CPACK_DEBIAN_PACKAGE_DEBUG ON)
            set(CPACK_DEBIAN_PACKAGE_GENERATE_SHLIBS ON)
        endif()
    endif(CPACK_GENERATOR MATCHES "DEB")

    set(CPACK_PACKAGE_EXECUTABLES   )
    set(CPACK_DISPLAY_NAME          ${USER_PROJECT_NAME})

    include(CPack)
endmacro()
