-- Sample Lua module. Load with: set lua_modules "example.lua"

modname = "example"
version = "1.0"

function et_InitGame(levelTime, randomSeed, restart)
    et.RegisterModname(modname .. " " .. version)
    et.G_Print("example.lua loaded\n")
end

function et_ClientCommand(clientNum, command)
    if string.lower(et.trap_Argv(0) or "") ~= "say" then
        return 0
    end
    local text = et.ConcatArgs(1) or ""
    if string.lower(string.sub(text, 1, 8)) ~= "!example" then
        return 0
    end
    et.trap_SendServerCommand(clientNum, "cpm \"example.lua is loaded.\"\n")
    return 1
end
