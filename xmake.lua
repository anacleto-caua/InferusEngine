-- Project Settings
add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = "./build/"})

set_project("Inferus Engine - The terrain thing.")
set_version("0.0.1")

-- Defaults to using clang and debug build
set_languages("c++23")
set_defaultmode("debug")

set_toolchains("clang")

local glfw_root = "libs/_libs_to_build/glfw"
target("glfw")
    add_defines(
        "_GLFW_WIN32",
        "_GLFW_VULKAN_STATIC",
        "_GLFW_NO_OPENGL",
        "_GLFW_NO_EGL",
        "_GLFW_NO_OSMESA",
        "_GLFW_NO_NULL"
    )
    set_kind("static")

    add_includedirs(path.join(glfw_root, "include"), {public = true})

    -- common
    add_files(
        path.join(glfw_root, "src/context.c"),
        path.join(glfw_root, "src/init.c"),
        path.join(glfw_root, "src/input.c"),
        path.join(glfw_root, "src/monitor.c"),
        path.join(glfw_root, "src/platform.c"),
        path.join(glfw_root, "src/vulkan.c"),
        path.join(glfw_root, "src/window.c")
    )

    if is_plat("windows") then
        add_defines("_GLFW_WIN32")
        add_files(
            path.join(glfw_root, "src/win32_*.c"),
            path.join(glfw_root, "src/wgl_context.c"),
            path.join(glfw_root, "src/egl_context.c"),
            path.join(glfw_root, "src/osmesa_context.c"),
            path.join(glfw_root, "src/null_*.c")
        )
        add_syslinks("user32", "gdi32", "shell32")
    elseif is_plat("linux") then
        -- I enable both X11 and Wayland, hopefully it wont bring me problems
        add_defines("_GLFW_X11", "_GLFW_WAYLAND")

        add_files(
            path.join(glfw_root, "src/x11_*.c"),
            path.join(glfw_root, "src/wl_*.c"),
            path.join(glfw_root, "src/glx_context.c"),
            path.join(glfw_root, "src/egl_context.c")
        )

        add_syslinks("X11", "wayland-client", "wayland-cursor", "wayland-egl", "pthread", "dl")
    end

-- Custom rule for shader compilation
rule("compile_shaders")
    set_extensions(".vert", ".frag", ".comp")
    on_build_file(function (target, sourcefile, opt)
        import("core.project.depend")
        import("lib.detect.find_program")
        local shader_name = path.filename(sourcefile)
        local output_file = path.join(target:targetdir(), "shaders", shader_name .. ".spv")

        local glslc = nil
        local vk_sdk = os.getenv("VULKAN_SDK")

        if vk_sdk then
            local ext = is_plat("windows") and ".exe" or ""
            glslc = path.join(vk_sdk, "bin", "glslc" .. ext)
        end

        if not glslc or not os.isfile(glslc) then
            glslc = find_program("glslc")
        end

        if glslc then
            depend.on_changed(function ()
                local outdir = path.directory(output_file)
                if not os.exists(outdir) then
                    os.mkdir(outdir)
                end
                if not os.exists(output_file) then
                    os.touch(output_file)
                end

                os.vrunv(glslc, {sourcefile, "-o", output_file})
                print("Compiling: " .. shader_name .. " -> " .. output_file)
            end, {files = sourcefile})
        else
            print("Warning: glslc not found. Skipping: " .. shader_name)
        end
    end)

-- Custom rule for coping assets
rule("copy_assets")
    on_build_file(function (target, sourcefile, opt)
        import("core.project.depend")
        local rel_path = path.relative(sourcefile, "resources")
        local dest_file = path.join(target:targetdir(), "resources", rel_path)
        depend.on_changed(function()
            os.cp(sourcefile, dest_file)
            print("Copying asset: " .. path.filename(sourcefile) .. " -> " .. dest_file)
        end, {files = sourcefile})
    end)

target("InferusEngine")
    set_kind("binary")
    set_default()

    add_deps("glfw")

    -- Generate debug files, keep symbols and disable optimazations
    set_symbols("debug")
    set_strip("none")
    set_optimize("none")

    set_warnings("all", "extra")
    add_cxflags("-Wpedantic")
    add_cxflags("-Wshadow")

    -- Treat third-party libs as system headers to suppress their warnings
    add_sysincludedirs("libs", "libs/vma", "libs/glm-1.0.2", "libs/spdlog/include", "libs/fnl", "libs/imgui", "libs/imgui/backends")

    -- Add source files
    add_files("src/**.cpp")
    add_includedirs("src")

    -- Include directories and set defines
    add_includedirs("src", "libs", "libs/vma", "libs/glm-1.0.2", "libs/spdlog/include", "libs/fnl", "libs/imgui", "libs/imgui/backends")

    add_files("libs/imgui/*.cpp")
    add_files("libs/imgui/backends/**.cpp")

    -- Global definitions
    add_defines(
        "GLM_FORCE_RADIANS",
        "GLM_FORCE_LEFT_HANDED",
        "GLM_FORCE_DEPTH_ZERO_TO_ONE",
        "GLM_ENABLE_EXPERIMENTAL",
        "GLFW_INCLUDE_VULKAN"
    )

    if is_plat("windows") then
        -- VULKAN (Env Var)
        local vk_sdk = os.getenv("VULKAN_SDK")
        if vk_sdk then
            add_sysincludedirs(path.join(vk_sdk, "Include"))
            add_linkdirs(path.join(vk_sdk, "Lib"))
        end
        add_syslinks("vulkan-1")

        add_syslinks("user32", "gdi32", "shell32")

    elseif is_plat("linux") then
        add_syslinks("vulkan")
        add_syslinks("dl", "pthread", "X11", "Xxf86vm", "Xrandr", "Xi")
    end

    -- Build Output Directory
    set_targetdir("build/$(plat)/$(mode)")

    -- Add shader and asset files to trigger the custom rules
    -- At the end, to avoid glslc's "No such file or directory"
    add_files("shaders/**.vert", "shaders/**.frag", "shaders/**.comp", {rule = "compile_shaders"})
    add_files("resources/**", {rule = "copy_assets"})

target_end()

-- Task to kick start rad debugger linked to project binary
task("rad")
    set_menu({
        usage = "xmake rad",
        description = "Builds the project and opens the RAD Debugger."
    })

    on_run(function ()
        import("core.project.config")
        import("core.project.project")

        config.load()
        os.exec("xmake")

        -- Find the binary target file
        local target_file = nil
        for name, target in pairs(project.targets()) do
            if target:kind() == "binary" then
                target_file = target:targetfile()
                break
            end
        end

        -- Launch
        if target_file then
            local native_path = path.translate(target_file)

            os.runv("raddbg.exe", {native_path}, {detach = true})
        else
            print("Error: Could not find a binary target.")
        end
    end)
task_end()
