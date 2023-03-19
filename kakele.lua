target("Kakele")
State = false
function KEY_8()
    -- print("KEY_8 started")
    State = not State
    input_key("KEY_8", State and 1 or 0)
    -- print("O resultado é: " .. tostring(State) .. "\n")
end

function BTN_RIGHT()
    -- print("BTN_RIGHT started")
    while get_key_state(273) do
        input_key_stroke(273)
        sleep(math.random(95, 128))
    end
    -- print("BTN_RIGHT released")
end
