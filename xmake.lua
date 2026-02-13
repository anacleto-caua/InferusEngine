-- Project Settings
add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = "./build/"})

set_project("Inferus Engine - The terrain thing.")
set_version("0.0.1")

-- Defaults to using clang and debug build
set_languages("c++26")
set_defaultmode("debug")

set_toolchains("clang")


-- Custom rule for shader compilation
rule("compile_shaders")
    set_extensions(".vert", ".frag", ".comp")
    on_build_file(function (target, sourcefile, opt)
        import("core.project.depend")
        local shader_name = path.filename(sourcefile)
        local output_file = path.join(target:targetdir(), "shaders", shader_name .. ".spv")
        -- Check env var, fallback to path
        local vk_sdk = os.getenv("VULKAN_SDK")
        local glslc = vk_sdk and path.join(vk_sdk, "bin", "glslc.exe") or find_tool("glslc")

        if glslc and os.isfile(glslc) then
            depend.on_changed(function ()
                os.vrunv(glslc, {sourcefile, "-o", output_file})
                print("Compiling: " .. shader_name .. " -> " .. output_file)
            end, {files = sourcefile})
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

    -- Generate debug files, keep symbols and disable optimazations
    set_symbols("debug")
    set_strip("none")
    set_optimize("none")

    -- Add source files
    add_files("src/**.cpp")

    -- Add shader and asset files to trigger the custom rules
    add_files("shaders/**.vert", "shaders/**.frag", "shaders/**.comp", {rule = "compile_shaders"})
    add_files("resources/**", {rule = "copy_assets"})

    -- Include directories and set defines
    add_includedirs("src", "libs/vma", "libs/glm-1.0.2", "libs/spdlog/include", "libs/fnl")
    add_defines("GLM_FORCE_RADIANS", "GLM_FORCE_DEPTH_ZERO_TO_ONE", "GLM_ENABLE_EXPERIMENTAL", "GLFW_INCLUDE_VULKAN")

    if is_plat("windows") then
        -- VULKAN (Env Var)
        local vk_sdk = os.getenv("VULKAN_SDK")
        if vk_sdk then
            add_includedirs(path.join(vk_sdk, "Include"))
            add_linkdirs(path.join(vk_sdk, "Lib"))
        end
        add_syslinks("vulkan-1")

        -- Find GLFW based on the GLFW_ROOT PATH variable - Author's Note: Install it alongside clang using Scoop
        local glfw_root = os.getenv("GLFW_ROOT")
        if not glfw_root or not os.isdir(glfw_root) then
            utils.error("Error: GLFW_ROOT environment variable is not set! Please point it to your GLFW folder.")
        end

        -- Add Include
        add_includedirs(path.join(glfw_root, "include"))

        -- Add Lib
        local lib_dir = path.join(glfw_root, "lib-vc2022")

        -- Fallback for older versions
        if not os.isdir(lib_dir) then lib_dir = path.join(glfw_root, "lib-vc2019") end
        if not os.isdir(lib_dir) then lib_dir = path.join(glfw_root, "lib-vc2017") end

        if os.isdir(lib_dir) then
            add_linkdirs(lib_dir)
            print("Linking GLFW from: " .. lib_dir)
        else
            utils.error("Error: Could not find a compatible 'lib-vcXXXX' folder in " .. glfw_root)
        end

        add_syslinks("glfw3")
        add_syslinks("user32", "gdi32", "shell32")

    elseif is_plat("linux") then
        add_syslinks("vulkan", "glfw")
        add_syslinks("dl", "pthread", "X11", "Xxf86vm", "Xrandr", "Xi")
    end

    -- Global definitions
    add_defines("GLM_FORCE_RADIANS", "GLM_FORCE_DEPTH_ZERO_TO_ONE", "GLM_ENABLE_EXPERIMENTAL", "GLFW_INCLUDE_VULKAN")

    -- Build Output Directory
    set_targetdir("build/$(plat)/$(mode)")

    -- Call RadDebugger -- disabled until I discover a way of not launching this every single time
    -- on_run(function (target)
        -- import("core.base.task")
        -- os.exec("raddbg.exe \"%s\"", target:targetfile())
    -- end)
