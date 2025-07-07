local M = {}

local ffi = require('ffi')

ffi.cdef[[
    int test_controller();
    int test_hook();
]]

local controller_dll
local hook_dll

local function load_native_libs()
    local plugin_path = debug.getinfo(1, "S").source:sub(2):match("(.*/)")
    
    -- Use vim.fn.resolve to properly resolve the relative path
    -- lua/medusa/ -> ../../bin/ (go up 2 levels to plugin root, then into bin)
    local bin_path = vim.fn.resolve(plugin_path .. "../../bin/")
    bin_path = bin_path:gsub("/", "\\") .. "\\"
    
    print("Medusa: Attempting to load from: " .. bin_path)
    
    -- Check if files exist
    local controller_path = bin_path .. "medusa_controller.dll"
    local hook_path = bin_path .. "medusa_hook.dll"
    
    if vim.fn.filereadable(controller_path) == 0 then
        error("Controller DLL not found at: " .. controller_path)
    end
    
    if vim.fn.filereadable(hook_path) == 0 then
        error("Hook DLL not found at: " .. hook_path)
    end
    
    controller_dll = ffi.load(controller_path)
    hook_dll = ffi.load(hook_path)
end

function M.setup(opts)
    opts = opts or {}
    
    local success, err = pcall(load_native_libs)
    if not success then
        print("Medusa: Failed to load native libraries: " .. err)
        return
    end
    
    print("Medusa: Plugin loaded successfully")
    
    local controller_result = controller_dll.test_controller()
    local hook_result = hook_dll.test_hook()
    
    print("Controller test result: " .. controller_result)
    print("Hook test result: " .. hook_result)
end

return M